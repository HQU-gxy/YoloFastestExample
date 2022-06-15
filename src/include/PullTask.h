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
    Options opts;
    bool isReadRedis = false;
    int  maxPoll = 1500;
    int  poll = 0;
    std::string pipeline;
    std::function<void(const int &)> onPollComplete = [](const int &) {};

    PullTask(CapProps capProps, Options opts, sw::redis::Redis &redis);
    void setVideoWriter(std::string pipe);

    void clearQueue();

    void run();
  };
}



#endif //YOLO_APP_PULLTASK_H
