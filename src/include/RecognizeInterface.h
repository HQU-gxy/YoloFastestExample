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
  class RecognizeInterface {
  private:
    std::function<void()> beforeRun = []() {};
    std::function<void()> afterRun = []() {};
    std::string type = "Unknown";
  protected:
    std::string filePath;

  public:
    inline RecognizeInterface(std::string filePath) : filePath(filePath) {
      spdlog::info("Input File Path: {}", filePath);
    }

    inline void setType(const std::string &type) {
      RecognizeInterface::type = type;
    }

    inline const std::string &getType() const {
      return type;
    }

    inline void setBeforeRunHook(const std::function<void()> &beforeRun) {
      RecognizeInterface::beforeRun = beforeRun;
    }

    inline void setAfterRunHook(const std::function<void()> &afterRun) {
      RecognizeInterface::afterRun = afterRun;
    }

    virtual int recognize(YoloFastestV2 &api, YoloApp::VideoOptions opts) = 0;

    virtual const std::shared_ptr<YoloApp::VideoHandler> &getVideoHandler() const = 0;

    int run(YoloFastestV2 &api, YoloApp::VideoOptions opts);
  };

  class Image : public RecognizeInterface {
  public:
    inline Image(const std::string inputFileName) : RecognizeInterface(inputFileName) {
      setType("Image");
    }
    inline const std::shared_ptr<YoloApp::VideoHandler> &getVideoHandler() const {
      return nullptr;
    };

    virtual int recognize(YoloFastestV2 &api, YoloApp::VideoOptions opts) override;
  };

  class Video : public RecognizeInterface {
  private:
    std::shared_ptr<YoloApp::VideoHandler> videoHandler;
  public:
    const std::shared_ptr<YoloApp::VideoHandler> &getVideoHandler() const;

    void setVideoHandler(const std::shared_ptr<YoloApp::VideoHandler> &videoHandler);

    inline Video(const std::string inputFileName) : RecognizeInterface(inputFileName) {
      setType("Video");
    }

    virtual int recognize(YoloFastestV2 &api, YoloApp::VideoOptions opts) override;
  };

  class Stream : public RecognizeInterface {
  private:
    std::shared_ptr<YoloApp::VideoHandler> videoHandler;

  public:
    const std::shared_ptr<YoloApp::VideoHandler> &getVideoHandler() const;

    void setVideoHandler(const std::shared_ptr<YoloApp::VideoHandler> &videoHandler);

    inline Stream(const std::string inputFileName) : RecognizeInterface(inputFileName) {
      setType("Stream");
    }

    virtual int recognize(YoloFastestV2 &api, YoloApp::VideoOptions opts) override;

  };

  std::unique_ptr<RecognizeInterface> createFile(const std::string &path);
}


#endif //YOLO_FASTESTV2_RECOGNIZEINTERFACE_H
