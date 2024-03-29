//
// Created by crosstyan on 2022/3/14.
//

#include "include/VideoInterface.h"

namespace YoloApp {

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
  VideoInterface::initializeVideoHandler(YoloFastestV2 &api, sw::redis::Redis &redis, Options &opts) {
    if (!cap.isOpened()) {
      spdlog::error("Cannot open video file");
      throw std::runtime_error("Cannot open video file");
    }
    if (videoHandler == nullptr) {
      auto ptr = std::make_shared<VideoHandler>(cap, api, redis, YoloApp::classNames, opts);
      spdlog::debug("Trying to initialize video handler at {:p}", fmt::ptr(ptr));
      videoHandler = ptr;
      return ptr;
    } else {
      return videoHandler;
    }
  }
}