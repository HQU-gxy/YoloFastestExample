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
    std::unique_ptr<YoloApp::PullTask> pullJob;
    std::unique_ptr<CapProps> capsProps;

  public:

    explicit MainWrapper(YoloApp::Options &opts);
    void init();
    std::thread pullRun();
    std::thread pushRun();

    Error setPullWriter(std::string pipeline);

    void pushRunDetach();
    void pullRunDetach();

    void setOnDetectDoor(const std::function<void(const std::vector<pt_pair> &)> &onDetectDoor);
    void setOnDetectYolo(const std::function<void(const std::vector<TargetBox> &)> &onDetectYolo);
    void setOnPollComplete(const std::function<void(int)> &onPollComplete);

    void setPullTaskState(bool isRunning);

    bool setMaxPoll(int max);

    bool getPullTaskState();

    void clearQueue();

    int getPoll();
    int getMaxPoll();

    void resetPoll();

    void enablePoll();
    void startPoll(std::string pipeline);

    int setCropRect(int x, int y, int w, int h);
    void setYoloState(bool isYolo);
  };

}



#endif //YOLO_FASTESTV2_MAINWRAPPER_H
