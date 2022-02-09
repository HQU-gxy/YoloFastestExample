//
// Created by crosstyan on 2022/2/9.
//

#include "include/detect.h"

bool IS_CAPTURE_ENABLED = true;

// not a pure function, will modify the cvImg
// @param classNames - the name of the class to be detected (array of strings)
std::vector<TargetBox> detectFrame(cv::Mat &cvImg, YoloFastestV2 &api, const std::vector<const char *> &classNames) {
  std::vector<TargetBox> boxes;

  api.detection(cvImg, boxes);

  for (TargetBox box: boxes) {
    char text[256];
    sprintf(text, "%s %.1f%%", classNames[box.cate], box.score * 100);

    int baseLine = 0;
    cv::Size label_size = cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);

    int x = box.x1;
    int y = box.y1 - label_size.height - baseLine;
    if (y < 0)
      y = 0;
    if (x + label_size.width > cvImg.cols)
      x = cvImg.cols - label_size.width;

    cv::rectangle(cvImg, cv::Rect(cv::Point(x, y), cv::Size(label_size.width, label_size.height + baseLine)),
                  cv::Scalar(255, 255, 255), -1);

    cv::putText(cvImg, text, cv::Point(x, y + label_size.height),
                cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));

    cv::rectangle(cvImg, cv::Point(box.x1, box.y1),
                  cv::Point(box.x2, box.y2), cv::Scalar(255, 255, 0), 2, 2, 0);
  }
  return boxes;
}

int handleVideo(cv::VideoCapture &cap, YoloFastestV2 &api, const std::vector<const char *> &classNames,
                const std::string &outputFileName, int codec, float scaledCoeffs) {

  int frame_width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
  int frame_height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
  int frame_fps = cap.get(cv::CAP_PROP_FPS);
  int frame_count = cap.get(cv::CAP_PROP_FRAME_COUNT);
  int real_frame_count = 0;

  spdlog::debug("Output video codec fourcc: 0x{0:x}", codec);
  cv::VideoWriter outputVideo(outputFileName, codec, frame_fps,
                              cv::Size(frame_width * scaledCoeffs, frame_height * scaledCoeffs));

  spdlog::debug("Original video size: {}x{}", frame_width, frame_height);
  spdlog::debug("Output video size: {}x{}", frame_width * scaledCoeffs, frame_height * scaledCoeffs);
  spdlog::debug("Original video fps: {}", frame_fps);
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
    auto boxes = detectFrame(cvImgResized, api, classNames);
    outputVideo.write(cvImgResized);
    auto end = ncnn::get_current_time();

    int key = cv::waitKey(1);
    if (key == 'q') {
      cv::destroyWindow("Stream");
      spdlog::info("q key is pressed by the user. Stopping the video");
      break;
    }

    real_frame_count++;
    if (frame_count > 0) {
      spdlog::info("[{}/{}]\t{} ms", real_frame_count, frame_count, end - start);
    } else {
      spdlog::info("[{}]\t{} ms", real_frame_count, end - start);
    }
  }
  return 0;
}
