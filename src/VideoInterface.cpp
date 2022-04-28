//
// Created by crosstyan on 2022/3/14.
//

#include "include/VideoInterface.h"
#include <memory>

namespace YoloApp {

  int VideoInterface::recognize(YoloFastestV2 &api, sw::redis::Redis &redis, YoloApp::Options opts) {
    auto writer = YoloApp::newVideoWriter(cap, opts,
                                                        YoloApp::base_pipeline +
                                                        opts.rtmpUrl);
    this->initializeVideoHandler(api, redis, opts)->run();
    return YoloApp::Error::SUCCESS;
  }

  std::optional<std::unique_ptr<VideoInterface>>
  createFile(const std::string &path) {
    auto type = getFileType(path);
    if (type == YoloApp::FileType::Video) {
      return std::optional(std::make_unique<Video>(path));
    } else if (type == YoloApp::FileType::Stream) {
      auto index = std::stoi(path);
      return std::optional(std::make_unique<Stream>(index));
    } else {
      return std::nullopt;
    }
  }

  std::shared_ptr<YoloApp::VideoHandler>
  VideoInterface::initializeVideoHandler(YoloFastestV2 &api, sw::redis::Redis &redis, Options opts) {
    if (!cap.isOpened()) {
      spdlog::error("Cannot open video file");
      throw std::runtime_error("Cannot open video file");
    }
    if (videoHandler == nullptr) {
      videoHandler = std::make_shared<VideoHandler>(cap, api, redis, YoloApp::classNames, opts);
    }
    return videoHandler;
  }
}