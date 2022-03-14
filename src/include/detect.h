//
// Created by crosstyan on 2022/2/9.
//

#ifndef YOLO_FASTESTV2_DETECT_H
#define YOLO_FASTESTV2_DETECT_H

#include "yolo-fastestv2.h"
#include <benchmark.h>
#include "spdlog/spdlog.h"

// Use namespace to avoid conflict with other libraries
// But define a namespace
namespace YoloApp {
  enum Error {
    SUCCESS = 0,
    FAILURE = 1
  };
  extern bool IS_CAPTURE_ENABLED;
  extern const std::vector<char const *> classNames;
  extern const std::string base_pipeline;
  struct VideoOptions {
    std::string outputFileName;
    std::string rtmpUrl;
    float scaledCoeffs = 1.0;
    // Maybe I should use the exact coordinate
    float cropCoeffs = 0.1;
    float outFps = 5;
    bool isRtmp = false;
    bool isDebug = true;
  };

  std::vector<TargetBox>
  detectFrame(cv::Mat &detectImg, cv::Mat &drawImg, YoloFastestV2 &api, const std::vector<const char *> &classNames);

  auto detectDoor(cv::Mat &detectImg, cv::Mat &drawImg, cv::Rect cropRect);

  int handleVideo(cv::VideoCapture &cap, YoloFastestV2 &api, const std::vector<const char *> &classNames,
                  const YoloApp::VideoOptions opts);

  // TODO: disable copy but enable move
  class VideoHandler {
  private:
    cv::VideoCapture &cap;
    YoloFastestV2 &api;
    cv::VideoWriter &video_writer;
    const std::vector<const char *> classNames;
    YoloApp::VideoOptions opts;
  public:
    bool isRedis = false;
    const VideoOptions &getOpts() const;

    void setOpts(const YoloApp::VideoOptions &opts);

    void setVideoWriter(cv::VideoWriter &writer);

    static cv::VideoWriter
    getInitialVideoWriter(cv::VideoCapture &cap, const YoloApp::VideoOptions opts, const std::string pipeline);

    VideoHandler(cv::VideoCapture &cap,
                 YoloFastestV2 &api,
                 cv::VideoWriter &writer,
                 const std::vector<const char *> classNames,
                 const YoloApp::VideoOptions opts);

    int run();
  };
}


#endif //YOLO_FASTESTV2_DETECT_H

