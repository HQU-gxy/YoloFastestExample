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
    r::Redis altRedis;
    std::unique_ptr<VideoInterface> recognize = nullptr;
    std::shared_ptr<VideoHandler> handler = nullptr;
    std::shared_ptr<YoloApp::PullTask> pullJob = nullptr;
    std::shared_ptr<YoloApp::PullTask> altPullJob = nullptr;

  private:
    std::shared_ptr<CapProps> capsProps = nullptr;

  public:
    explicit MainWrapper(YoloApp::Options &opts);
    const std::shared_ptr<YoloApp::PullTask> &getAltPullJob() const;
    const std::shared_ptr<VideoHandler> &getHandler() const;
    const std::shared_ptr<YoloApp::PullTask> & getPullJob() const;

    void init();
    std::thread pullRun();
    std::thread altPullRun();
    std::thread pushRun();

    void pushRunDetach();
    void pullRunDetach();
    void altPullRunDetach();
  };

}



#endif //YOLO_FASTESTV2_MAINWRAPPER_H
