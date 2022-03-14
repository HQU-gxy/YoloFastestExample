//
// Created by crosstyan on 2022/3/14.
//

#include "include/RecognizeInterface.h"
#include <memory>

namespace YoloApp {

  int Video::recognize(YoloFastestV2 &api, sw::redis::Redis &redis, YoloApp::VideoOptions opts) {
    if (opts.outputFileName.empty()) {
      opts.outputFileName = getOutputFileName(filePath);
    }
    cv::VideoCapture cap(filePath);
    if (!cap.isOpened()) {
      spdlog::error("Cannot open video file");
      return YoloApp::Error::FAILURE;
    }
    auto writer = YoloApp::VideoHandler::getInitialVideoWriter(cap, opts,
                                                               YoloApp::base_pipeline +
                                                               opts.rtmpUrl);
    YoloApp::VideoHandler handler{cap, api, writer, redis, YoloApp::classNames, opts};
    videoHandler = std::make_shared<YoloApp::VideoHandler>(handler);
    handler.run();
    return YoloApp::Error::SUCCESS;
  }

  int Stream::recognize(YoloFastestV2 &api, sw::redis::Redis &redis, YoloApp::VideoOptions opts) {
    auto index = std::stoi(filePath);
    spdlog::info("Streaming from camera {}", index);
    // I don't output the video to file for stream
    cv::VideoCapture cap(index);
    if (!cap.isOpened()) {
      spdlog::error("Cannot open video file");
      return YoloApp::Error::FAILURE;
    }
    auto writer = YoloApp::VideoHandler::getInitialVideoWriter(cap, opts,
                                                               YoloApp::base_pipeline +
                                                               opts.rtmpUrl);
    YoloApp::VideoHandler handler{cap, api, writer, redis, YoloApp::classNames, opts};
    videoHandler = std::make_shared<YoloApp::VideoHandler>(handler);
    handler.run();
    return YoloApp::Error::SUCCESS;
  }

  const std::shared_ptr<YoloApp::VideoHandler> Stream::getVideoHandler() const {
    return videoHandler;
  }

  const std::shared_ptr<YoloApp::VideoHandler> Video::getVideoHandler() const {
    return videoHandler;
  }

  // Will throw exception if the file is not a valid file
  std::unique_ptr<RecognizeInterface> createFile(const std::string &path, sw::redis::Redis& redis) {
    auto type = getFileType(path);
    if (type == YoloApp::FileType::Video) {
      return std::make_unique<Video>(path, redis);
    } else if (type == YoloApp::FileType::Stream) {
      return std::make_unique<Stream>(path, redis);
    } else {
      throw std::runtime_error("Unknown file type");
    }
  }
}