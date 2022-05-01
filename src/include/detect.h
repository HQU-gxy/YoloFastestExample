//
// Created by crosstyan on 2022/2/9.
//

#ifndef YOLO_FASTESTV2_DETECT_H
#define YOLO_FASTESTV2_DETECT_H

#include "yolo-fastestv2.h"
#include <benchmark.h>
#include <sw/redis++/redis++.h>
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
  struct Options {
    std::string outputFileName;
    std::string rtmpUrl;
    float scaledCoeffs = 1.0;
    // Maybe I should use the exact coordinate
    float cropCoeffs = 0.1;
    float outFps = 5;
    bool isDebug = false;
  };

  struct CapProps {
    const double frame_width;
    const double frame_height;
    const double frame_fps;
  };

  std::vector<TargetBox>
  detectFrame(cv::Mat &detectImg,
              cv::Mat &drawImg,
              YoloFastestV2 &api,
              const std::vector<const char *> &classNames,
              const std::function<void(const std::vector<TargetBox> &)>& cb);

  using pt = std::tuple<int, int>;
  using pt_pair = std::tuple<pt, pt>;

  auto detectDoor(cv::Mat &detectImg,
                  cv::Mat &drawImg,
                  cv::Rect cropRect,
                  const std::function<void(const std::vector<pt_pair> &)>& cb);

  // TODO: disable copy but enable move
  class VideoHandler {
  private:
    std::function<void(const std::vector<pt_pair> &)> onDetectDoor = [](const std::vector<pt_pair> &) {};
    std::function<void(const std::vector<TargetBox> &)> onDetectYolo = [](const std::vector<TargetBox> &) {};
//    std::function<void(const std::string &)> onError = [](const std::string &) {};
//    std::function<void(const std::string &)> onInfo = [](const std::string &) {};
    cv::VideoCapture &cap;
    YoloFastestV2 &api;
//    cv::VideoWriter &video_writer;
  public:
    void setOnDetectDoor(const std::function<void(const std::vector<pt_pair> &)> &onDetectDoor);

    void setOnDetectYolo(const std::function<void(const std::vector<TargetBox> &)> &onDetectYolo);

    sw::redis::Redis &redis;
    const std::vector<const char *> classNames;
    YoloApp::Options opts;
    bool isWriteRedis = true;
  public:
    VideoHandler(cv::VideoCapture &cap, YoloFastestV2 &api, sw::redis::Redis &redis,
                 const std::vector<const char *> classNames, const YoloApp::Options opts);

    int run();

  };

  class PullTask {
    std::unique_ptr<cv::VideoWriter> writer = nullptr;
    sw::redis::Redis &redis;
  public:
    CapProps capProps;
    Options opts;
    bool isReadRedis = false;
    int  maxPoll = 1500;
    int  poll = 0;
    std::string pipeline;
    std::function<void(const int &)> onPollComplete = [](const int &) {};

    PullTask(CapProps capProps, Options opts, sw::redis::Redis &redis);
    void setVideoWriter(std::string pipeline);

    void clearQueue();

    void run();
  };

  YoloApp::CapProps getCapProps(cv::VideoCapture &cap);
}


#endif //YOLO_FASTESTV2_DETECT_H

