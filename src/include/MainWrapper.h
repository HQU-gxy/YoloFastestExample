//
// Created by crosstyan on 2022/4/26.
//

#ifndef YOLO_FASTESTV2_MAINWRAPPER_H
#define YOLO_FASTESTV2_MAINWRAPPER_H

#ifdef _STANDALONE_ON
#include <csignal>
#endif
#include <future>
#include <functional>
#include "PullTask.h"
#include "VideoInterface.h"

namespace r = sw::redis;

namespace YoloApp::Main {
  class MainWrapper {
  private:
    YoloFastestV2 api;
    YoloApp::Options options;
    Options &opts;
    r::Redis pullRedis;
    r::Redis pushRedis;
    std::unique_ptr<VideoInterface> recognize;
    std::shared_ptr<VideoHandler> handler;
    std::shared_ptr<YoloApp::PullTask> pullJob;
    std::unique_ptr<CapProps> capsProps;

  public:
    explicit MainWrapper(YoloApp::Options &opts);
    const std::shared_ptr<VideoHandler> &getHandler() const;
    const std::shared_ptr<YoloApp::PullTask> & getPullJob() const;

    void init();
    std::thread pullRun();
    std::thread pushRun();

    void pushRunDetach();
    void pullRunDetach();
  };

}



#endif //YOLO_FASTESTV2_MAINWRAPPER_H
