//
// Created by crosstyan on 2022/3/14.
//

#ifndef YOLO_FASTESTV2_RECOGNIZEINTERFACE_H
#define YOLO_FASTESTV2_RECOGNIZEINTERFACE_H

#include <functional>
#include <sw/redis++/redis++.h>
#include <string>
#include "utils.h"
#include "detect.h"

namespace YoloApp {
  // TODO: disable copy but enable move
  class RecognizeInterface {
  protected:
    std::string type = "Unknown";
    std::string filePath;
    std::shared_ptr<YoloApp::VideoHandler> videoHandler = nullptr;

    inline void setType(const std::string &type) {
      RecognizeInterface::type = type;
    }

  public:
    inline RecognizeInterface(std::string filePath) : filePath(filePath) {
      spdlog::debug("Input File Path: {}", filePath);
    }

    inline const std::string &getType() const {
      return type;
    }

    inline CapProps getCapProps(Options opts) {
      auto caps = this->getCap(opts);
      auto capsProps = YoloApp::VideoHandler::getCapProps(caps);
      caps.release(); // release the caps to prevent memory leak
      return capsProps;
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

    virtual int recognize(YoloFastestV2 &api, sw::redis::Redis &redis, YoloApp::Options opts) = 0;

    virtual cv::VideoCapture getCap(YoloApp::Options opts) = 0;
  };

  class Video : public RecognizeInterface {
  public:
    inline Video(const std::string inputFileName) : RecognizeInterface(inputFileName) {
      setType("Video");
      spdlog::debug("Input File is: {}", type);
    }

    virtual cv::VideoCapture getCap(YoloApp::Options opts) override;

    virtual int recognize(YoloFastestV2 &api, sw::redis::Redis &redis, YoloApp::Options opts) override;
  };

  class Stream : public RecognizeInterface {
  public:
    inline Stream(const std::string inputFileName) : RecognizeInterface(inputFileName) {
      setType("Stream");
      spdlog::info("Input File is: {}", type);
    }

    virtual int recognize(YoloFastestV2 &api, sw::redis::Redis &redis, YoloApp::Options opts) override;

    virtual cv::VideoCapture getCap(YoloApp::Options opts) override;

  };

  std::unique_ptr<RecognizeInterface> createFile(const std::string &path);
}


#endif //YOLO_FASTESTV2_RECOGNIZEINTERFACE_H
