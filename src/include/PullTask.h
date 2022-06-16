//
// Created by crosstyan on 2022/6/15.
//

#ifndef YOLO_APP_PULLTASK_H
#define YOLO_APP_PULLTASK_H

#include "detect.h"
namespace YoloApp {
  class PullTask {
    std::unique_ptr<cv::VideoWriter> writer = nullptr;
    sw::redis::Redis &redis;
  public:
    CapProps capProps;
    Options &opts;
    bool isReadRedis = false;
    int  maxPoll = 1500;
    int  poll = 0;
    std::string pipeline;
    std::string redisKey;
    std::function<void(const int &)> onPollComplete = [](const int &) {};


    PullTask(std::string key, sw::redis::Redis &redis, CapProps capProps, Options &opts);

    void setVideoWriter(std::string pipe);
    void setOnPollComplete(const std::function<void(const int &)> &onPollComplete);
    void startPoll(std::string pipeline);
    void resetPoll();
    void clearQueue();
    void run();
  };
}



#endif //YOLO_APP_PULLTASK_H
