//
// Created by crosstyan on 2022/2/9.
//

#include "include/detect.h"
#include "date.h"
#include "math.h"

bool IS_CAPTURE_ENABLED = true;

// not a pure function, will modify the detectImg
// @param classNames - the name of the class to be detected (array of strings)
std::vector<TargetBox>
detectFrame(cv::Mat &detectImg, cv::Mat &drawImg, YoloFastestV2 &api, const std::vector<const char *> &classNames) {
  std::vector<TargetBox> boxes;

  api.detection(detectImg, boxes);

  for (TargetBox box: boxes) {
    char text[256];
    sprintf(text, "%s %.1f%%", classNames[box.cate], box.score * 100);

    int baseLine = 0;
    cv::Size label_size = cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);

    int x = box.x1;
    int y = box.y1 - label_size.height - baseLine;
    if (y < 0)
      y = 0;
    if (x + label_size.width > detectImg.cols)
      x = detectImg.cols - label_size.width;

    cv::rectangle(drawImg, cv::Rect(cv::Point(x, y), cv::Size(label_size.width, label_size.height + baseLine)),
                  cv::Scalar(255, 255, 255), -1);

    cv::putText(drawImg, text, cv::Point(x, y + label_size.height),
                cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));

    cv::rectangle(drawImg, cv::Point(box.x1, box.y1),
                  cv::Point(box.x2, box.y2), cv::Scalar(255, 255, 0), 2, 2, 0);
  }
  return boxes;
}

auto drawTime(cv::Mat &drawImg) {
  // add current time
  auto now = std::chrono::system_clock::now();
  auto formatted = date::format("%Y-%m-%d %H:%M:%S", now);
  cv::putText(drawImg, formatted.c_str(), cv::Point(10, 20), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 1,
              cv::LINE_AA);
}

auto detectDoor(cv::Mat &detectImg, cv::Mat &drawImg, cv::Rect cropRect) {
  cv::Mat processedImg;
  cv::Mat edges;
  cv::Mat lines;

  cv::GaussianBlur(detectImg, processedImg, cv::Size(5, 5), 0, 0);
  cv::Canny(processedImg, edges, 100, 200);
  cv::HoughLinesP(edges, lines, 0.5, CV_PI / 180, 30, 50, 10);
  if (!lines.empty()){
    auto newDrawImg = drawImg(cropRect);
    lines.forEach<cv::Vec4i>([&](cv::Vec4i &line, const int *position) {
      auto x1 = line[0];
      auto y1 = line[1];
      auto x2 = line[2];
      auto y2 = line[3];
      auto angle = abs(atan2(y2 - y1, x2 - x1) * 180 / CV_PI);
      if (angle > 85 && angle < 95) {
        cv::line(newDrawImg, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(255, 0, 0), 2, cv::LINE_AA);
      } else {
        cv::line(newDrawImg, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 0, 255), 2, cv::LINE_AA);
      }
    });
  }
}

int handleVideo(cv::VideoCapture &cap, YoloFastestV2 &api, const std::vector<const char *> &classNames,
                const std::string &outputFileName, const std::string &rtmpUrl, float scaledCoeffs, float outFps) {
  const int frame_width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
  const int frame_height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
  const int frame_fps = cap.get(cv::CAP_PROP_FPS);
  const int frame_count = cap.get(cv::CAP_PROP_FRAME_COUNT);
  int real_frame_count = 0;
  const auto CROP_COEF = 0.1;

  if (outFps == 0) {
    outFps = frame_fps;
  }

  // apiReference using gstreamer
  // fourcc 0 means uncompressed
  // using OpenCV's Gstreamer API
  // gst-launch-1.0 -v videotestsrc ! x264enc ! flvmux ! rtmpsink location='rtmp://localhost:1935/live/rfBd56ti2SMtYvSgD5xAV0YU99zampta7Z7S575KLkIZ9PYk'
  // gst-launch-1.0 videotestsrc is-live=true ! x264enc  pass=5 quantizer=25 speed-preset=6 ! video/x-h264, profile=baseline  ! flvmux ! rtmpsink location='rtmp://localhost:1935/live/rfBd56ti2SMtYvSgD5xAV0YU99zampta7Z7S575KLkIZ9PYk'
  // gst-inspect-1.0 | grep x264
  // ffmpeg -re -i demo.flv -c copy -f flv rtmp://localhost:1935/live/rfBd56ti2SMtYvSgD5xAV0YU99zampta7Z7S575KLkIZ9PYk

  const std::string pipeline = "appsrc ! "
                               "videoconvert ! "
                               "x264enc  pass=5 quantizer=25 speed-preset=6 ! "
                               "video/x-h264, profile=baseline ! "
                               "flvmux ! "
                               "rtmpsink location=" + rtmpUrl;
  cv::VideoWriter outputVideo(pipeline, cv::CAP_GSTREAMER, 0, outFps,
                              cv::Size(frame_width * scaledCoeffs, frame_height * scaledCoeffs));

  spdlog::debug("Original video size: {}x{}", frame_width, frame_height);
  spdlog::debug("Output video size: {}x{}", frame_width * scaledCoeffs, frame_height * scaledCoeffs);
  spdlog::debug("Original video fps: {}", frame_fps);
  spdlog::debug("Output video fps: {}", outFps);
  spdlog::debug("Original video frame count: {}", frame_count);
  while (IS_CAPTURE_ENABLED) {
    cv::Mat cvImg;
    cv::Mat cvImgResized;
    cap >> cvImg;
    if (cvImg.empty()) {
      break;
    }
    auto start = ncnn::get_current_time();
    cv::resize(cvImg, cvImgResized, cv::Size(frame_width * scaledCoeffs, frame_height * scaledCoeffs));

    const auto vertical_middle = frame_width * scaledCoeffs / 2;
    const auto crop_right_pt = vertical_middle + (frame_width * scaledCoeffs * CROP_COEF);
    const auto crop_left_pt = vertical_middle - (frame_width * scaledCoeffs * CROP_COEF);
    const auto length = crop_right_pt - crop_left_pt;
    auto cropRect = cv::Rect(crop_left_pt, 0, length, length);
    auto origImg = cvImgResized.clone();
    auto croppedImg = cvImgResized(cropRect);

    detectDoor(croppedImg, cvImgResized, cropRect);
    cv::rectangle(cvImgResized,cropRect, cv::Scalar(0, 204, 255), 2, cv::LINE_AA);
    auto boxes = detectFrame(origImg, cvImgResized, api, classNames);
    drawTime(cvImgResized);
    outputVideo.write(cvImgResized);
    auto end = ncnn::get_current_time();

    real_frame_count++;
    if (frame_count > 0) {
      spdlog::info("[{}/{}]\t{} ms", real_frame_count, frame_count, end - start);
    } else {
      spdlog::info("[{}]\t{} ms", real_frame_count, end - start);
    }
  }
  return 0;
}
