//
// Created by crosstyan on 2022/4/26.
//

#ifndef YOLO_FASTESTV2_MAINWRAPPER_H
#define YOLO_FASTESTV2_MAINWRAPPER_H

#include <csignal>
#include <future>
#include <functional>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include "detect.h"
#include "VideoInterface.h"

namespace py = pybind11;
namespace r = sw::redis;

namespace YoloApp::Main {

  struct Options {
    std::string inputFilePath;
    std::string outputFileName;
    std::string paramPath;
    std::string binPath;
    std::string rtmpUrl;
    std::string redisUrl = "tcp://127.0.0.1:6379";
    float scaledCoeffs = 1.0;
    float thresholdNMS = 0.1;
    float outFps = 0.0;
    float cropCoeffs = 0.1;
    int threadsNum = 4;
    bool isDebug = false;
  };

  class MainWrapper {
  private:
    YoloFastestV2 api;
    YoloApp::Options options;
    Options opts;
    r::Redis pullRedis;
    r::Redis pushRedis;
    YoloApp::Options videoOpts;
    std::unique_ptr<VideoInterface> recognize;
    std::shared_ptr<VideoHandler> handler;
    std::unique_ptr<YoloApp::PullTask> pullJob;
    std::unique_ptr<CapProps> capsProps;

  public:
    MainWrapper(const Options &opts);
    void init();
    std::thread pullRun();
    std::thread pushRun();

    Error swapPullWriter(std::string pipeline);

    const Options &getOpts() const;

    void pushRunDetach();
    void pullRunDetach();

    void setOnDetectDoor(const std::function<void(const std::vector<pt_pair> &)> &onDetectDoor);
    void setOnDetectYolo(const std::function<void(const std::vector<TargetBox> &)> &onDetectYolo);
  };

  static YoloApp::Options toVideoOptions(const Options &opts);
  Options OptionsFromPyDict(const py::dict &dict);
}



#endif //YOLO_FASTESTV2_MAINWRAPPER_H
