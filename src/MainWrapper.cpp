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
  this->pullJob = std::make_unique<YoloApp::PullTask>(*capsProps, opts, this->pullRedis);
}

y::Error m::MainWrapper::setPullWriter(std::string pipeline) {
  try {
    spdlog::info("Swap writer to pipeline {}", pipeline);
    if (this->pullJob == nullptr || this->capsProps == nullptr) {
      throw std::runtime_error("pull thread or capsProps is uninitialized");
    }
    this->pullJob->setVideoWriter(pipeline);
    return y::SUCCESS;
  } catch (std::exception &e) {
    spdlog::error(e.what());
    return y::Error::FAILURE;
  }
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

void
YoloApp::Main::MainWrapper::setOnDetectDoor
    (const std::function<void(const std::vector<pt_pair> &)> &onDetectDoor) {
  if (this->handler == nullptr) {
    throw std::runtime_error("handler is uninitialized");
  }
  this->handler->setOnDetectDoor(onDetectDoor);
}

void
YoloApp::Main::MainWrapper::setOnDetectYolo
    (const std::function<void(const std::vector<TargetBox> &)> &onDetectYolo) {
  if (this->handler == nullptr) {
    throw std::runtime_error("handler is uninitialized");
  }
  this->handler->setOnDetectYolo(onDetectYolo);
}

void YoloApp::Main::MainWrapper::setPullTaskState(bool isRunning) {
  if (this->pullJob == nullptr) {
    throw std::runtime_error("pullJob is uninitialized");
  }
  this->pullJob->isReadRedis = isRunning;
}

bool YoloApp::Main::MainWrapper::getPullTaskState() {
  if (this->pullJob == nullptr) {
    throw std::runtime_error("pullJob is uninitialized");
  }
  return this->pullJob->isReadRedis;
}

bool m::MainWrapper::setMaxPoll(int max) {
  if (this->pullJob == nullptr) {
    throw std::runtime_error("pullJob is uninitialized");
  }
  if (max < 0) {
    return false;
  }
  this->pullJob->maxPoll = max;
  return true;
}

// set the pull job writer to pipeline and stop the pull job
void m::MainWrapper::resetPoll() {
  if (this->pullJob == nullptr) {
    throw std::runtime_error("pullJob is uninitialized");
  }
  this->setPullWriter("");
  this->pullJob->poll = 0;
  this->pullJob->isReadRedis = false;
}

int m::MainWrapper::getMaxPoll() {
  if (this->pullJob == nullptr) {
    throw std::runtime_error("pullJob is uninitialized");
  }
  return this->pullJob->maxPoll;
}

int m::MainWrapper::getPoll() {
  if (this->pullJob == nullptr) {
    throw std::runtime_error("pullJob is uninitialized");
  }
  return this->pullJob->poll;
}

void m::MainWrapper::enablePoll() {
  if (this->pullJob == nullptr) {
    throw std::runtime_error("pullJob is uninitialized");
  }
  this->pullJob->isReadRedis = true;
}

void m::MainWrapper::setOnPollComplete(const std::function<void(int)> &onPollComplete) {
  this->pullJob->onPollComplete = onPollComplete;
}

void YoloApp::Main::MainWrapper::startPoll(std::string pipeline) {
  if (this->pullJob == nullptr) {
    throw std::runtime_error("pullJob is uninitialized");
  }
  this->setPullWriter(pipeline);
  this->enablePoll();
}

void YoloApp::Main::MainWrapper::clearQueue() {
  if (this->pullJob == nullptr) {
    throw std::runtime_error("pullJob is uninitialized");
  }
  this->pullJob->clearQueue();
}

int YoloApp::Main::MainWrapper::setCropRect(int x, int y, int w, int h) {
  if (this->handler == nullptr) {
    throw std::runtime_error("Handler is uninitialized");
  }
  return this->handler->setCropRect(x, y, w, h);
}

void YoloApp::Main::MainWrapper::setYoloState(bool isYolo) {
  if (this->handler == nullptr) {
    throw std::runtime_error("Handler is uninitialized");
  }
  this->handler->isYolo = isYolo;
}





