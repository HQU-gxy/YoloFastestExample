//
// Created by crosstyan on 2022/2/9.
//

#ifndef YOLO_FASTESTV2_DETECT_H
#define YOLO_FASTESTV2_DETECT_H

#include "yolo-fastestv2.h"
#include <benchmark.h>
#include <Config.h>
#include <sw/redis++/redis++.h>
#include <pybind11/pybind11.h>
#include <cmath>
#include "spdlog/spdlog.h"
#include "date.h"

namespace YoloApp {
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
    void saveToRedis(cv::Mat image, std::string key);
    void setOnDetectDoor(const std::function<void(const std::vector<pt_pair> &)> &onDetectDoor);
    void setOnDetectYolo(const std::function<void(const std::vector<TargetBox> &)> &onDetectYolo);

    sw::redis::Redis &redis;
    std::vector<const char *> classNames;
    YoloApp::Options &opts;
    bool isYolo = true;

    VideoHandler(cv::VideoCapture &cap, YoloFastestV2 &api, sw::redis::Redis &redis,
                 const std::vector<const char *> classNames, Options &opts);
    int run();
  };


  YoloApp::CapProps getCapProps(cv::VideoCapture &cap);
}


#endif //YOLO_FASTESTV2_DETECT_H

