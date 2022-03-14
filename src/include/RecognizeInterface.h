//
// Created by crosstyan on 2022/3/14.
//

#ifndef YOLO_FASTESTV2_RECOGNIZEINTERFACE_H
#define YOLO_FASTESTV2_RECOGNIZEINTERFACE_H

#include <functional>
#include <string>
#include "utils.h"
#include "detect.h"

namespace YoloApp {
  // TODO: disable copy but enable move
  class RecognizeInterface {
  protected:
    std::string type = "Unknown";
    std::string filePath;

  public:
    inline RecognizeInterface(std::string filePath) : filePath(filePath) {
      spdlog::debug("Input File Path: {}", filePath);
    }

    inline void setType(const std::string &type) {
      RecognizeInterface::type = type;
    }

    inline const std::string &getType() const {
      return type;
    }

    virtual int recognize(YoloFastestV2 &api, YoloApp::VideoOptions opts) = 0;

    // shared_ptr and unique_ptr are designed to pass by value
    virtual const std::shared_ptr<YoloApp::VideoHandler> getVideoHandler() const = 0;

    int run(YoloFastestV2 &api, YoloApp::VideoOptions opts);
  };

  class Image : public RecognizeInterface {
  public:
    inline Image(const std::string inputFileName) : RecognizeInterface(inputFileName) {
      setType("Image");
      spdlog::debug("Input File is: {}", type);
    }

    inline const std::shared_ptr<YoloApp::VideoHandler> getVideoHandler() const {
      return nullptr;
    };

    virtual int recognize(YoloFastestV2 &api, YoloApp::VideoOptions opts) override;
  };

  class Video : public RecognizeInterface {
  private:
    std::shared_ptr<YoloApp::VideoHandler> videoHandler;
  public:
    const std::shared_ptr<YoloApp::VideoHandler> getVideoHandler() const;

    inline Video(const std::string inputFileName) : RecognizeInterface(inputFileName) {
      setType("Video");
      spdlog::debug("Input File is: {}", type);
    }

    virtual int recognize(YoloFastestV2 &api, YoloApp::VideoOptions opts) override;
  };

  class Stream : public RecognizeInterface {
  private:
    std::shared_ptr<YoloApp::VideoHandler> videoHandler;

  public:
    const std::shared_ptr<YoloApp::VideoHandler> getVideoHandler() const;

    inline Stream(const std::string inputFileName) : RecognizeInterface(inputFileName) {
      setType("Stream");
      spdlog::info("Input File is: {}", type);
    }

    virtual int recognize(YoloFastestV2 &api, YoloApp::VideoOptions opts) override;

  };

  std::unique_ptr<RecognizeInterface> createFile(const std::string &path);
}


#endif //YOLO_FASTESTV2_RECOGNIZEINTERFACE_H
