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
    sw::redis::Redis& redis;
    std::string type = "Unknown";
    std::string filePath;

  public:
    inline RecognizeInterface(std::string filePath, sw::redis::Redis& redis) : filePath(filePath), redis{redis} {
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
  };

  class Video : public RecognizeInterface {
  private:
    std::shared_ptr<YoloApp::VideoHandler> videoHandler;
  public:
    const std::shared_ptr<YoloApp::VideoHandler> getVideoHandler() const;

    inline Video(const std::string inputFileName, sw::redis::Redis& redis) : RecognizeInterface(inputFileName, redis) {
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

    Stream(const std::string inputFileName, sw::redis::Redis& redis);

    virtual int recognize(YoloFastestV2 &api, YoloApp::VideoOptions opts) override;

  };

  std::unique_ptr<RecognizeInterface> createFile(const std::string &path, sw::redis::Redis& redis);
}


#endif //YOLO_FASTESTV2_RECOGNIZEINTERFACE_H
