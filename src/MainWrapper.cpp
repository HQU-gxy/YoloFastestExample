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
  this->capsProps = std::make_shared<CapProps>(handler->getCapProps());
  this->pullJob = std::make_shared<YoloApp::PullTask>(opts.cacheKey, this->pullRedis, *capsProps, opts);
  if (opts.isSaveAlt) {
    this->altPullJob = std::make_shared<YoloApp::PullTask>(opts.altCacheKey, this->altRedis, *capsProps, opts);
  }
  spdlog::debug("Pointers: handler at {}, PullJob at {}", fmt::ptr(this->handler), fmt::ptr(this->pullJob));
}

std::thread m::MainWrapper::pushRun() {
  if (this->handler == nullptr) {
    throw std::runtime_error("handler is uninitialized");
  }
  std::thread pushTask([=]() {
    this->handler->run();
  });
  return pushTask;
}

std::thread m::MainWrapper::pullRun() {
  if (this->pullJob == nullptr) {
    throw std::runtime_error("pullJob is uninitialized");
  }
  std::thread pullTask([=]() {
    pullJob->run();
  });
  return pullTask;
}

std::thread YoloApp::Main::MainWrapper::altPullRun() {
  if (this->altPullJob == nullptr) {
    throw std::runtime_error("altPullJob is uninitialized");
  }
  std::thread pullTask([=]() {
    altPullJob->run();
  });
  return pullTask;
}

// TODO: refactor this to eliminate duplicated code
void m::MainWrapper::pushRunDetach() {
  this->pushRun().detach();
}

void m::MainWrapper::pullRunDetach() {
  this->pullRun().detach();
}

void YoloApp::Main::MainWrapper::altPullRunDetach() {
  this->altPullRun().detach();
}

m::MainWrapper::MainWrapper(y::Options &opts)
    : api(YoloFastestV2(opts.threadsNum, opts.thresholdNMS)),
      opts(opts),
      pullRedis(opts.redisUrl),
      pushRedis(opts.redisUrl),
      altRedis(opts.redisUrl){
  api.loadModel(opts.paramPath.c_str(), opts.binPath.c_str());
}

const std::shared_ptr<y::VideoHandler> &YoloApp::Main::MainWrapper::getHandler() const {
  if (handler == nullptr){
    throw std::runtime_error("handler not initialized");
  } else {
    return handler;
  }
}

const std::shared_ptr<YoloApp::PullTask> & YoloApp::Main::MainWrapper::getPullJob() const {
  if (pullJob == nullptr) {
    throw std::runtime_error("pull job not initialized");
  } else {
    return pullJob;
  }
}

const std::shared_ptr<YoloApp::PullTask> &YoloApp::Main::MainWrapper::getAltPullJob() const {
  if (altPullJob == nullptr) {
    throw std::runtime_error("pull job not initialized");
  } else {
    return altPullJob;
  }
}






