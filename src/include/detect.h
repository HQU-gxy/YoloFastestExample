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
  detectFrame(cv::Mat &detectImg, cv::Mat &drawImg, YoloFastestV2 &api, const std::vector<const char *> &classNames);

  auto detectDoor(cv::Mat &detectImg, cv::Mat &drawImg, cv::Rect cropRect);

  // TODO: disable copy but enable move
  class VideoHandler {
  private:
    std::function<void(const std::string &)> onDetectYolo = [](const std::string &) {};
    std::function<void(const std::string &)> onDetectDoor = [](const std::string &) {};
    std::function<void(const std::string &)> onError =  [](const std::string &) {};
    std::function<void(const std::string &)> onInfo =  [](const std::string &) {};
    cv::VideoCapture &cap;
    YoloFastestV2 &api;
    cv::VideoWriter &video_writer;
  public:
    sw::redis::Redis &redis;
    const std::vector<const char *> classNames;
    YoloApp::Options opts;
    bool isWriteRedis = true;
    bool isWriteVideoWriter = false;
  public:
    cv::VideoWriter getVideoWriter() const;

    const Options &getOpts() const;

    void setOpts(const YoloApp::Options &opts);

    void setVideoWriter(cv::VideoWriter writer);

    static cv::VideoWriter
    newVideoWriter(cv::VideoCapture &cap, const YoloApp::Options opts, const std::string pipeline);

    static cv::VideoWriter
    newVideoWriter(YoloApp::CapProps props, const YoloApp::Options opts,
                   const std::string pipeline);


    static YoloApp::CapProps getCapProps(cv::VideoCapture &cap);

    VideoHandler(cv::VideoCapture &cap, YoloFastestV2 &api, cv::VideoWriter &writer, sw::redis::Redis &redis,
                 const std::vector<const char *> classNames, const YoloApp::Options opts);

    int run();

    void setVideoWriter(const std::string &pipeline);
  };
  class PullTask {
  private:
    cv::VideoWriter writer;
  public:
    bool isReadRedis = true;
    void setVideoWriter(cv::VideoWriter writer);
    PullTask(cv::VideoWriter writer);
    PullTask(std::string pipeline);
    void run(Options opts, sw::redis::Redis& redis);
  };
}


#endif //YOLO_FASTESTV2_DETECT_H

