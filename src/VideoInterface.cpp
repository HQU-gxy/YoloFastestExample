//
// Created by crosstyan on 2022/3/14.
//

#include "include/VideoInterface.h"
#include <memory>

namespace YoloApp {

  int VideoInterface::recognize(YoloFastestV2 &api, sw::redis::Redis &redis, YoloApp::Options opts) {
    auto writer = YoloApp::VideoHandler::getInitialVideoWriter(cap, opts,
                                                               YoloApp::base_pipeline +
                                                               opts.rtmpUrl);
    this->initializeVideoHandler(api, redis, writer, opts)->run();
    return YoloApp::Error::SUCCESS;
  }

  //! Will throw exception if the file is not a valid file
  // TODO: use optional
  std::unique_ptr<VideoInterface> createFile(const std::string &path) {
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