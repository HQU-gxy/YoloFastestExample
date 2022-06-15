//
// Created by crosstyan on 2022/3/14.
//

#ifndef YOLO_FASTESTV2_VIDEOINTERFACE_H
#define YOLO_FASTESTV2_VIDEOINTERFACE_H

#include <functional>
#include <sw/redis++/redis++.h>
#include "utils.h"
#include "detect.h"

namespace YoloApp {
  // TODO: disable copy but enable move
  class VideoInterface {
  protected:
    std::string type = "Unknown";
    std::string filePath;
    cv::VideoCapture cap;
    std::shared_ptr<YoloApp::VideoHandler> videoHandler;

  public:
    std::shared_ptr<YoloApp::VideoHandler>
    initializeVideoHandler(YoloFastestV2 &api, sw::redis::Redis &redis, Options &opts);

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
      return YoloApp::getCapProps(cap);
    }

    // shared_ptr and unique_ptr are designed to pass by value
    //! Danger this function can't be used before call recognize
    //! before which videoHandler is not initialized
    inline const std::optional<std::shared_ptr<YoloApp::VideoHandler>> getVideoHandler() const {
      return (videoHandler != nullptr) ? std::optional(videoHandler) : std::nullopt;
    }

  };

  // TODO: handle output file name differently
  class Video : public VideoInterface {
  public:
    // TODO: Video writes to writer directly
    // Don't write to redis
    // No idea why segfault
    inline Video(const std::string inputFileName) : VideoInterface(inputFileName) {
      this->type = "Video";
      spdlog::debug("Input File is {}", type);
      this->cap = cv::VideoCapture(filePath);
    }
  };

  class Stream : public VideoInterface {
  public:
    inline Stream(const int index) : VideoInterface(std::to_string(index)) {
      this->type = "Stream";
      spdlog::info("Streaming from camera {}", index);
      // I don't output the video to file for stream
      this->cap = cv::VideoCapture(index);
    }
  };

  std::optional<std::unique_ptr<VideoInterface>> createFile(const std::string &path);
}


#endif //YOLO_FASTESTV2_VIDEOINTERFACE_H
