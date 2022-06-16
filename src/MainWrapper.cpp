// Created by crosstyan on 2022/4/26.
//

#include "MainWrapper.h"

namespace m = YoloApp::Main;
namespace r = sw::redis;
namespace y = YoloApp;

void m::MainWrapper::init() {
  #ifdef _STANDALONE_ON
  // Ctrl + C
  // Use Signal Sign to tell the application to stop
  signal(SIGINT, [](int sig) {
    spdlog::error("SIGINT is received. Force stopping the application");
    exit(1);
  });


  // Ctrl + Z
  // Don't just use exit() or OpenCV won't save the video correctly
  signal(SIGTSTP, [](int sig) {
    spdlog::warn("SIGTSTP is received. Stopping capture");
    YoloApp::IS_CAPTURE_ENABLED = false;
  });
  #endif

  if (opts.isDebug) {
    spdlog::set_level(spdlog::level::debug);
  }

  this->recognize = YoloApp::createFile(opts.inputFilePath).value();
  this->handler = this->recognize->initializeVideoHandler(api, pushRedis, opts);
  this->capsProps = std::make_unique<CapProps>(handler->getCapProps());
  this->pullJob = std::make_shared<YoloApp::PullTask>(opts.cacheKey, this->pullRedis, *capsProps, opts);
}

std::thread m::MainWrapper::pushRun() {
  if (this->recognize == nullptr) {
    throw std::runtime_error("recognize is uninitialized");
  }
  std::thread pushTask([&]() {
    this->handler->run();
  });
  return pushTask;
}

void m::MainWrapper::pushRunDetach() {
  this->pushRun().detach();
}

void m::MainWrapper::pullRunDetach() {
  this->pullRun().detach();
}

std::thread m::MainWrapper::pullRun() {
  if (this->pullJob == nullptr) {
    throw std::runtime_error("recognize is uninitialized");
  }
  std::thread pullTask([&]() {
    pullJob->run();
  });
  return pullTask;
}

m::MainWrapper::MainWrapper(y::Options &opts)
    : api(YoloFastestV2(opts.threadsNum, opts.thresholdNMS)),
      opts(opts),
      pullRedis(opts.redisUrl),
      pushRedis(opts.redisUrl) {
  api.loadModel(opts.paramPath.c_str(), opts.binPath.c_str());
}

const std::shared_ptr<y::VideoHandler> &YoloApp::Main::MainWrapper::getHandler() const {
  return handler;
}

const std::shared_ptr<YoloApp::PullTask> & YoloApp::Main::MainWrapper::getPullJob() const {
  return pullJob;
}





