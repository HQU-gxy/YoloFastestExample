//
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
  this->handler = this->recognize->initializeVideoHandler(api, pushRedis, videoOpts);
  this->capsProps = std::make_unique<CapProps>(recognize->getCapProps());
  auto writer =
      YoloApp::newVideoWriter(*capsProps, videoOpts, YoloApp::base_pipeline + opts.rtmpUrl);
  // make sure the parameter of PullTask is by value
  // or RAII will release writer and cause problems.
  this-> pullJob = std::make_unique<YoloApp::PullTask>(YoloApp::PullTask(writer));
}

y::Error m::MainWrapper::swapPullWriter(std::string pipeline){
  try {
    spdlog::info("Swap writer to pipeline {}", pipeline);
    if (this->pullJob == nullptr || this->capsProps == nullptr) {
      throw std::runtime_error("pull thread or capsProps is uninitialized");
    }
    auto writer =
        YoloApp::newVideoWriter(*capsProps, videoOpts, pipeline);
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
  std::thread pullTask([&](){
    pullJob->run(this->videoOpts, this->pullRedis);
  });
  return pullTask;
}

#ifndef _STANDALONE_ON
m::Options m::OptionsFromPyDict(const py::dict &dict){
  auto opts = YoloApp::Main::Options{
      .inputFilePath = dict["input_file_path"].cast<std::string>(),
      .outputFileName = dict["output_file_path"].cast<std::string>(),
      .paramPath = dict["param_path"].cast<std::string>(),
      .binPath = dict["bin_path"].cast<std::string>(),
      .rtmpUrl = dict["rtmp_url"].cast<std::string>(),
      .redisUrl = dict["redis_url"].cast<std::string>(),
      .scaledCoeffs = dict["scaled_coeffs"].cast<float>(),
      .thresholdNMS = dict["threshold_NMS"].cast<float>(),
      .outFps = dict["out_fps"].cast<float>(),
      .cropCoeffs = dict["crop_coeffs"].cast<float>(),
      .threadsNum = dict["threads_num"].cast<int>(),
      .isDebug = dict["is_debug"].cast<bool>(),
  };
  return opts;
}
#endif

m::MainWrapper::MainWrapper(const m::Options &opts)
    : opts(opts),
      pullRedis(opts.redisUrl),
      pushRedis(opts.redisUrl),
      api(YoloFastestV2(opts.threadsNum, opts.thresholdNMS)),
      videoOpts(m::toVideoOptions(opts)) {
  api.loadModel(opts.paramPath.c_str(), opts.binPath.c_str());
}

const YoloApp::Main::Options &YoloApp::Main::MainWrapper::getOpts() const {
  return opts;
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




