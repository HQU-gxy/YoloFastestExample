//
// Created by crosstyan on 2022/4/26.
//

#include "MainWrapper.h"

namespace m = YoloApp::Main;
namespace r = sw::redis;
namespace y = YoloApp;

void m::MainWrapper::init() {

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

  if (opts.isDebug) {
    spdlog::set_level(spdlog::level::debug);
  }

  this->recognize = YoloApp::createFile(opts.inputFilePath).value();

  this->capsProps = std::make_unique<CapProps>(recognize->getCapProps());
  auto writer =
      YoloApp::VideoHandler::newVideoWriter(*capsProps, videoOpts, YoloApp::base_pipeline + opts.rtmpUrl);
  // make sure the parameter of PullTask is by value
  // or RAII will release writer and cause problems.
  this-> pullJob = std::make_unique<YoloApp::PullTask>(YoloApp::PullTask(writer));
}

y::Error m::MainWrapper::swapPushWriter(std::string pipeline){
  try {
    if (this->recognize == nullptr) {
      throw std::runtime_error("push thread is uninitialized");
    }
    this->recognize->getVideoHandler().value()->setVideoWriter(pipeline);
    return y::SUCCESS;
  } catch (std::exception e){
    spdlog::error(e.what());
    return y::Error::FAILURE;
  }
}

y::Error m::MainWrapper::swapPullWriter(std::string pipeline){
  try {
    if (this->pullJob == nullptr || this->capsProps == nullptr) {
      throw std::runtime_error("pull thread or capsProps is uninitialized");
    }
    auto writer =
        YoloApp::VideoHandler::newVideoWriter(*capsProps, videoOpts, pipeline);
    this->pullJob->setVideoWriter(writer);
    return y::SUCCESS;
  } catch (std::exception e){
    spdlog::error(e.what());
    return y::Error::FAILURE;
  }
}

std::thread m::MainWrapper::pushRun() {
  if (this->recognize == nullptr) {
    throw std::runtime_error("recognize is uninitialized");
  }
  std::thread pushTask([&](){
    this->recognize->recognize(this->api, this->pushRedis, this->videoOpts);
  });
  return pushTask;
}

std::thread m::MainWrapper::pullRun() {
  if (this->pullJob == nullptr) {
    throw std::runtime_error("recognize is uninitialized");
  }
  std::thread pullTask([&](){
    pullJob->run(this->videoOpts, this->pullRedis);
  });
  return pullTask;
}

m::MainWrapper::MainWrapper(const m::Options &opts)
    : opts(opts),
      pullRedis(opts.redisUrl),
      pushRedis(opts.redisUrl),
      api(YoloFastestV2(opts.threadsNum, opts.thresholdNMS)),
      videoOpts(m::toVideoOptions(opts)) {
  api.loadModel(opts.paramPath.c_str(), opts.binPath.c_str());
}

y::Options m::toVideoOptions(const m::Options &opts){
  YoloApp::Options vopts{
      .outputFileName = opts.outputFileName,
      .rtmpUrl = opts.rtmpUrl,
      .scaledCoeffs = opts.scaledCoeffs,
      .cropCoeffs = opts.cropCoeffs,
      .outFps = opts.outFps,
      .isDebug = opts.isDebug,
  };
  return vopts;
}
