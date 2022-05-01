//
// Created by crosstyan on 2022/4/26.
//

#ifndef YOLO_FASTESTV2_MAINWRAPPER_H
#define YOLO_FASTESTV2_MAINWRAPPER_H

#include <csignal>
#include <future>
#include <functional>
#include "detect.h"
#include "VideoInterface.h"

#ifndef _STANDALONE_ON
#include <pybind11/pybind11.h>
namespace py = pybind11;
#endif

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

    Error setPullWriter(std::string pipeline);

    void pushRunDetach();
    void pullRunDetach();

    void setOnDetectDoor(const std::function<void(const std::vector<pt_pair> &)> &onDetectDoor);
    void setOnDetectYolo(const std::function<void(const std::vector<TargetBox> &)> &onDetectYolo);

    void setPullTaskState(bool isRunning);

    bool setMaxPoll(int max);

    bool getPullTaskState();

    void clearQueue();

    int getPoll();
    int getMaxPoll();

    void setOnPollComplete(const std::function<void(int)> &onPollComplete);

    void resetPoll();

    void enablePoll();
    void startPoll(std::string pipeline);
  };

  static YoloApp::Options toVideoOptions(const Options &opts);

  #ifndef _STANDALONE_ON
  Options OptionsFromPyDict(const py::dict &dict);
  #endif
}



#endif //YOLO_FASTESTV2_MAINWRAPPER_H
