//
// Created by crosstyan on 2022/3/14.
//

#include "include/RecognizeInterface.h"
#include <memory>

namespace YoloApp {


  cv::VideoCapture Video::getCap() {
    return cv::VideoCapture(filePath);
  };

  cv::VideoCapture Stream::getCap() {
    auto index = std::stoi(filePath);
    // I don't output the video to file for stream
    return cv::VideoCapture(index);
  };

  int Video::recognize(YoloFastestV2 &api, sw::redis::Redis &redis, YoloApp::Options opts) {
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

  int Stream::recognize(YoloFastestV2 &api, sw::redis::Redis &redis, YoloApp::Options opts) {
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

  //! Will throw exception if the file is not a valid file
  // TODO: use optional
  std::unique_ptr<RecognizeInterface> createFile(const std::string &path) {
    auto type = getFileType(path);
    if (type == YoloApp::FileType::Video) {
      return std::make_unique<Video>(path);
    } else if (type == YoloApp::FileType::Stream) {
      return std::make_unique<Stream>(path);
    } else {
      throw std::runtime_error("Unknown file type");
    }
  }
}