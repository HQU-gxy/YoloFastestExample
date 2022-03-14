//
// Created by crosstyan on 2022/2/9.
//

#include "include/detect.h"
#include "date.h"
#include "math.h"

using namespace YoloApp;

// a global flag in order to make signal function work
bool YoloApp::IS_CAPTURE_ENABLED = true;
const std::vector<char const *> YoloApp::classNames = {
    "person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck", "boat", "traffic light",
    "fire hydrant", "stop sign", "parking meter", "bench", "bird", "cat", "dog", "horse", "sheep", "cow",
    "elephant", "bear", "zebra", "giraffe", "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee",
    "skis", "snowboard", "sports ball", "kite", "baseball bat", "baseball glove", "skateboard", "surfboard",
    "tennis racket", "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl", "banana", "apple",
    "sandwich", "orange", "broccoli", "carrot", "hot dog", "pizza", "donut", "cake", "chair", "couch",
    "potted plant", "bed", "dining table", "toilet", "tv", "laptop", "mouse", "remote", "keyboard", "cell phone",
    "microwave", "oven", "toaster", "sink", "refrigerator", "book", "clock", "vase", "scissors", "teddy bear",
    "hair drier", "toothbrush"
};
const std::string YoloApp::base_pipeline = "appsrc ! "
                                           "videoconvert ! "
                                           "x264enc  pass=5 quantizer=25 speed-preset=6 ! "
                                           "video/x-h264, profile=baseline ! "
                                           "flvmux ! "
                                           "rtmpsink location=";

// not a pure function, will modify the drawImg
// @param classNames - the name of the class to be detected (array of strings)
std::vector<TargetBox>
YoloApp::detectFrame(cv::Mat &detectImg, cv::Mat &drawImg, YoloFastestV2 &api,
                     const std::vector<const char *> &classNames) {
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

auto YoloApp::detectDoor(cv::Mat &detectImg, cv::Mat &drawImg, cv::Rect cropRect) {
  cv::Mat processedImg;
  cv::Mat edges;
  cv::Mat lines;

  cv::GaussianBlur(detectImg, processedImg, cv::Size(5, 5), 0, 0);
  cv::Canny(processedImg, edges, 100, 200);
  cv::HoughLinesP(edges, lines, 0.5, CV_PI / 180, 30, 50, 10);
  if (!lines.empty()) {
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

void VideoHandler::setVideoWriter(cv::VideoWriter &writer) {
  auto original_writer = this->video_writer;
  this->video_writer = writer;
  original_writer.release();
}

void VideoHandler::setOpts(const YoloApp::VideoOptions &opts) {
  VideoHandler::opts = opts;
}

cv::VideoWriter
VideoHandler::getInitialVideoWriter(cv::VideoCapture &cap, const YoloApp::VideoOptions opts,
                                    const std::string pipeline) {
  const int frame_width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
  const int frame_height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
  const int frame_fps = cap.get(cv::CAP_PROP_FPS);
  auto out_framerate = opts.outFps == 0 ? frame_fps : opts.outFps;
  return cv::VideoWriter(pipeline, cv::CAP_GSTREAMER, 0, out_framerate,
                         cv::Size(frame_width * opts.scaledCoeffs, frame_height * opts.scaledCoeffs));
}

cv::VideoWriter
VideoHandler::getInitialVideoWriter(YoloApp::CapProps props, const YoloApp::VideoOptions opts,
                                    const std::string pipeline) {
  auto [frame_width, frame_height, frame_fps] = props;
  auto out_framerate = opts.outFps == 0 ? frame_fps : opts.outFps;
  return cv::VideoWriter(pipeline, cv::CAP_GSTREAMER, 0, out_framerate,
                         cv::Size(frame_width * opts.scaledCoeffs, frame_height * opts.scaledCoeffs));
}


YoloApp::CapProps VideoHandler::getCapProps(cv::VideoCapture &cap) {
  return {
      cap.get(cv::CAP_PROP_FRAME_WIDTH),
      cap.get(cv::CAP_PROP_FRAME_HEIGHT),
      cap.get(cv::CAP_PROP_FPS),
  };
}

// I should move the ownership of cap and YoloFastestV2 API to VideoHandler
VideoHandler::VideoHandler(cv::VideoCapture &cap, YoloFastestV2 &api, cv::VideoWriter &writer, sw::redis::Redis &redis,
                           const std::vector<const char *> classNames, const YoloApp::VideoOptions opts)
    : cap{cap}, api{api}, classNames{classNames}, redis{redis}, opts{opts}, video_writer{writer} {}

int VideoHandler::run() {
  const int frame_width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
  const int frame_height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
  const int frame_fps = cap.get(cv::CAP_PROP_FPS);
  const int frame_count = cap.get(cv::CAP_PROP_FRAME_COUNT);
  int real_frame_count = 0;
  const auto CROP_COEF = opts.cropCoeffs;

  auto out_framerate = opts.outFps == 0 ? frame_fps : opts.outFps;

  spdlog::debug("Original video size: {}x{}", frame_width, frame_height);
  spdlog::debug("Output video size: {}x{}", frame_width * opts.scaledCoeffs, frame_height * opts.scaledCoeffs);
  spdlog::debug("Original video fps: {}", frame_fps);
  spdlog::debug("Output video fps: {}", out_framerate);
  spdlog::debug("Original video frame count: {}", frame_count);
  redis.del("image");
  while (YoloApp::IS_CAPTURE_ENABLED) {
    cv::Mat cvImg;
    cv::Mat cvImgResized;
    cap >> cvImg;
    if (cvImg.empty()) {
      break;
    }
    auto start = ncnn::get_current_time();
    cv::resize(cvImg, cvImgResized, cv::Size(frame_width * opts.scaledCoeffs, frame_height * opts.scaledCoeffs));

    const auto vertical_middle = frame_width * opts.scaledCoeffs / 2;
    const auto crop_right_pt = vertical_middle + (frame_width * opts.scaledCoeffs * CROP_COEF);
    const auto crop_left_pt = vertical_middle - (frame_width * opts.scaledCoeffs * CROP_COEF);
    const auto length = crop_right_pt - crop_left_pt;
    auto cropRect = cv::Rect(crop_left_pt, 0, length, length);
    auto origImg = cvImgResized.clone();
    auto croppedImg = cvImgResized(cropRect);

    detectDoor(croppedImg, cvImgResized, cropRect);
    cv::rectangle(cvImgResized, cropRect, cv::Scalar(0, 204, 255), 1, cv::LINE_AA);
    auto boxes = detectFrame(origImg, cvImgResized, api, classNames);
    drawTime(cvImgResized);

    if (opts.isRedis) {
      // uchar = unsigned char
      std::vector<uchar> buf;
      auto success = cv::imencode(".png", cvImgResized, buf);
      spdlog::debug("Send Vector Length: {}", buf.size());
      if (success) {
        auto len = redis.llen("image");
        if (len < 1500) {
//          spdlog::debug("Redis List length: {}", len);
          redis.lpush("image", reinterpret_cast<char *>(buf.data()));
        } else {
          redis.rpop("image");
          redis.lpush("image", reinterpret_cast<char *>(buf.data()));
        }
      } else {
        spdlog::error("Failed to encode image");
        throw std::runtime_error("Failed to encode image");
      }
    } else {
      video_writer.write(cvImgResized);
    }

    auto end = ncnn::get_current_time();

    real_frame_count++;
    if (frame_count > 0) {
      spdlog::info("[{}/{}]\t{} ms", real_frame_count, frame_count, end - start);
    } else {
      spdlog::info("[{}]\t{} ms", real_frame_count, end - start);
    }
  }
  return YoloApp::Error::SUCCESS;
}

PullTask::PullTask(cv::VideoWriter &writer) : writer{writer} {}

void PullTask::run(VideoOptions opts, sw::redis::Redis &redis) {
  while (YoloApp::IS_CAPTURE_ENABLED) {
    auto start = ncnn::get_current_time();
    //  sw::redis::OptionalStringPair redisMemory = redis.brpop("image", 0);
    auto redisMemory = redis.brpop("image", 0)
        .value_or(std::make_pair("", ""))
        .second;
    if (redisMemory.empty()) {
      continue;
    }
    auto vector = std::vector<uchar>(redisMemory.begin(), redisMemory.end());
    spdlog::debug("Received Vector Length: {}", vector.size());
    auto image = cv::imdecode(vector, cv::IMREAD_COLOR);
    if (image.empty()) {
      spdlog::error("Failed to decode image");
      throw std::runtime_error("Failed to decode image");
    }
    writer.write(image);
    auto end = ncnn::get_current_time();
    spdlog::info("[Pull]\t{} ms", end - start);
  }
}

void PullTask::setVideoWriter(cv::VideoWriter &writer) {
  PullTask::writer = writer;
}

const VideoOptions &VideoHandler::getOpts() const {
  return opts;
}

// I expected to copy
cv::VideoWriter VideoHandler::getVideoWriter() const {
  auto temp = video_writer;
  return temp;
}
