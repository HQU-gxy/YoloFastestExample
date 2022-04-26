//
// Created by crosstyan on 2022/4/26.
//

#ifndef YOLO_FASTESTV2_MAINWRAPPER_H
#define YOLO_FASTESTV2_MAINWRAPPER_H

#include <csignal>
#include <future>
#include "detect.h"
#include "VideoInterface.h"

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
    YoloFastestV2 api;
    YoloApp::Options options;
    Options opts;
    r::Redis pullRedis;
    r::Redis pushRedis;
    YoloApp::Options videoOpts;
    std::unique_ptr<VideoInterface> recognize;
    std::unique_ptr<YoloApp::PullTask> pullJob;

  public:
    MainWrapper(const Options &opts);
    void init();
    std::thread pullRun();
    std::thread pushRun();

    void blockPullRun();

    void initPull();
  };

  YoloApp::Options toVideoOptions(const Options &opts);
}



#endif //YOLO_FASTESTV2_MAINWRAPPER_H
