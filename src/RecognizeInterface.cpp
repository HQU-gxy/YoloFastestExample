//
// Created by crosstyan on 2022/3/14.
//

#include "include/RecognizeInterface.h"
#include <memory>

namespace YoloApp {
  int RecognizeInterface::run(YoloFastestV2 &api, YoloApp::VideoOptions opts) {
    recognize(api, opts);
    return 0;
  }

  int Image::recognize(YoloFastestV2 &api, YoloApp::VideoOptions opts) {
    if (opts.outputFileName.empty()) {
      opts.outputFileName = getOutputFileName(filePath);
    }
    cv::Mat cvImg = cv::imread(filePath);
    auto boxes = YoloApp::detectFrame(cvImg, cvImg, api, YoloApp::classNames);
    if (opts.isDebug) {
      // spdlog::debug("{}\t{}\t{}\t{}\t{}\t{}", "x1", "y1", "x2", "y2", "score", "class");
      for (TargetBox box: boxes) {
        spdlog::debug("{}\t{}\t{}\t{}\t{}\t{}", box.x1, box.y1, box.x2, box.y2, box.score,
                      YoloApp::classNames[box.cate]);
      }
    }
    cv::imwrite(opts.outputFileName, cvImg);
    return YoloApp::Error::SUCCESS;
  }

  int Video::recognize(YoloFastestV2 &api, YoloApp::VideoOptions opts) {
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
    YoloApp::VideoHandler handler{cap, api, writer,
                                  YoloApp::classNames, opts};
    videoHandler = std::make_shared<YoloApp::VideoHandler>(handler);
    handler.run();
    return YoloApp::Error::SUCCESS;
  }

  int Stream::recognize(YoloFastestV2 &api, YoloApp::VideoOptions opts) {
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
    YoloApp::VideoHandler handler{cap, api, writer,
                                  YoloApp::classNames, opts};
    videoHandler = std::make_shared<YoloApp::VideoHandler>(handler);
    handler.run();
    return YoloApp::Error::SUCCESS;
  }

  const std::shared_ptr<YoloApp::VideoHandler> &Stream::getVideoHandler() const {
    return videoHandler;
  }

  const std::shared_ptr<YoloApp::VideoHandler> &Video::getVideoHandler() const {
    return videoHandler;
  }

  std::unique_ptr<RecognizeInterface> createFile(const std::string &path) {
    auto type = getFileType(path);
    if (type == YoloApp::FileType::Image) {
      return std::make_unique<Image>(path);
    } else if (type == YoloApp::FileType::Video) {
      return std::make_unique<Video>(path);
    } else if (type == YoloApp::FileType::Stream) {
      return std::make_unique<Stream>(path);
    } else {
      throw std::runtime_error("Unknown file type");
    }
  }
}