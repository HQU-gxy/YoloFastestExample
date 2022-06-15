//
// Created by crosstyan on 2022/2/9.
//

#ifndef YOLO_FASTESTV2_DETECT_H
#define YOLO_FASTESTV2_DETECT_H

#include "yolo-fastestv2.h"
#include <benchmark.h>
#include <sw/redis++/redis++.h>
#include <pybind11/pybind11.h>
#include "spdlog/spdlog.h"
#include "date.h"
#include <cmath>
#include <vector>

namespace YoloApp {
  enum Error {
    SUCCESS = 0,
    FAILURE = 1
  };

  extern bool IS_CAPTURE_ENABLED;
  extern const std::vector<char const *> classNames;

  // This shit should be global variable/singleton. Maybe.
  struct Options {
    float scaledCoeffs = 1.0;
    int targetInputWidth = -1; // target means MAYBE the value will be set
    int targetInputHeight = -1; // maybe not
    float targetInputFPS = -1;
    float outputFPS = 5; // Should be set if input is Web Camera, but not if input is video file (set the value to a negative number)
    bool isBorder = true; // if true, display elevator door detect border
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
    cv::VideoCapture &cap;
    YoloFastestV2 &api;

    int frame_width;
    int frame_height;
    int frame_fps;
    int frame_count;
    cv::Rect cropRect;
  public:
    YoloApp::CapProps getCapProps();

    int setCropRect(int x, int y, int w, int h);
    void setOnDetectDoor(const std::function<void(const std::vector<pt_pair> &)> &onDetectDoor);
    void setOnDetectYolo(const std::function<void(const std::vector<TargetBox> &)> &onDetectYolo);

    sw::redis::Redis &redis;
    const std::vector<const char *> classNames;
    YoloApp::Options opts;
    bool isWriteRedis = true;
    bool isYolo = true;

    VideoHandler(cv::VideoCapture &cap, YoloFastestV2 &api, sw::redis::Redis &redis,
                 const std::vector<const char *> classNames, const YoloApp::Options opts);
    int run();
  };


  YoloApp::CapProps getCapProps(cv::VideoCapture &cap);
}


#endif //YOLO_FASTESTV2_DETECT_H

