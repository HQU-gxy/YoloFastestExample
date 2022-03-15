//
// Created by crosstyan on 2022/3/14.
//

#ifndef YOLO_FASTESTV2_VIDEOINTERFACE_H
#define YOLO_FASTESTV2_VIDEOINTERFACE_H

#include <functional>
#include <sw/redis++/redis++.h>
#include <string>
#include "utils.h"
#include "detect.h"

namespace YoloApp {
  // TODO: disable copy but enable move
  class VideoInterface {
  protected:
    std::string type = "Unknown";
    std::string filePath;
    cv::VideoCapture cap;
    std::shared_ptr<YoloApp::VideoHandler> videoHandler = nullptr;

    inline void setType(const std::string &type) {
      VideoInterface::type = type;
    }

  public:
    inline VideoInterface(std::string filePath) : filePath(filePath) {
      spdlog::debug("Input File Path: {}", filePath);
    }

    inline const std::string &getType() const {
      return type;
    }

    inline const cv::VideoCapture &getCap() const {
      return cap;
    }

    inline CapProps getCapProps() {
      return YoloApp::VideoHandler::getCapProps(cap);
    }

    inline std::shared_ptr<YoloApp::VideoHandler>
    initializeVideoHandler(YoloFastestV2 &api, sw::redis::Redis &redis, cv::VideoWriter &writer, Options opts) {
      if (!cap.isOpened()) {
        spdlog::error("Cannot open video file");
        throw std::runtime_error("Cannot open video file");
      }
      videoHandler = std::make_shared<VideoHandler>(cap, api, writer, redis, YoloApp::classNames, opts);
      return videoHandler;
    }

    // shared_ptr and unique_ptr are designed to pass by value
    //! Danger this function can't be used before call recognize
    //! before which videoHandler is not initialized
    inline const std::optional<std::shared_ptr<YoloApp::VideoHandler>> getVideoHandler() const {
      if (videoHandler) {
        return videoHandler;
      } else {
        return std::nullopt;
      }
    }

    int recognize(YoloFastestV2 &api, sw::redis::Redis &redis, YoloApp::Options opts);
  };

  // TODO: handle output file name differently
  class Video : public VideoInterface {
  public:
    inline Video(const std::string inputFileName) : VideoInterface(inputFileName) {
      setType("Video");
      spdlog::debug("Input File is {}", type);
      this->cap = cv::VideoCapture(filePath);
    }
  };

  class Stream : public VideoInterface {
  public:
    inline Stream(const std::string inputFileName) : VideoInterface(inputFileName) {
      setType("Stream");
      auto index = std::stoi(filePath);
      spdlog::info("Streaming from camera {}", index);
      // I don't output the video to file for stream
      this->cap = cv::VideoCapture(index);
    }
  };

  std::unique_ptr<VideoInterface> createFile(const std::string &path);
}


#endif //YOLO_FASTESTV2_VIDEOINTERFACE_H
