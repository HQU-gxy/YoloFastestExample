//
// Created by crosstyan on 2022/2/9.
//

#include "include/utils.h"

namespace YoloApp {
  FileType getFileType(const std::string &fileName) noexcept {
    std::filesystem::path inputPath(fileName);
    auto inputExtension = inputPath.extension().string();
    // convert to lower case
    std::for_each(inputExtension.begin(), inputExtension.end(), [](char &c) {
      c = ::tolower(c);
    });
    spdlog::debug("Input file extension: {}", inputExtension);
    if (exists(inputPath)) {
      if (inputExtension == ".jpg" || inputExtension == ".jpeg" || inputExtension == ".png") {
        if (exists(inputPath)) {}
        return FileType::Image;
      } else if (inputExtension == ".mp4" || inputExtension == ".avi" || inputExtension == ".mov" ||
                 inputExtension == ".mkv") {
        return FileType::Video;
      }
    } else {
      spdlog::warn("Input file {} does not exist. Using input as camera index. ", fileName);
      try {
        auto index = std::stoi(fileName);
        if (index >= 0) {
          return FileType::Stream;
        }
      } catch (std::exception &e) {
        spdlog::error("Error: {}", e.what());
        spdlog::error("Invalid input {}", fileName);
        return FileType::Unknown;
      }
      // assume it is a device id can be used as index of cv::VideoCapture
      return FileType::Stream;
    }
    return FileType::Unknown;
  }


  int getCodec(const std::string &codec) noexcept{
    if (codec == "mjpeg") {
      return cv::VideoWriter::fourcc('M', 'J', 'P', 'G');
    } else if (codec == "x264") {
      return cv::VideoWriter::fourcc('X', '2', '6', '4');
    } else if (codec == "h264") {
      return cv::VideoWriter::fourcc('H', '2', '6', '4');
    } else if (codec == "mp4v") {
      return cv::VideoWriter::fourcc('m', 'p', '4', 'v');
    } else if (codec == "mkvh") {
      return cv::VideoWriter::fourcc('m', 'k', 'v', 'h');
    } else {
      spdlog::warn("Unknown codec: {}, using mp4v", codec);
      return cv::VideoWriter::fourcc('m', 'p', '4', 'v');
    }
  }

  std::string getOutputFileName(const std::string &inputFileName, const std::string postFix) noexcept{
    std::filesystem::path inputPath(inputFileName);
    return inputPath.stem().string() + postFix + inputPath.extension().string();
  }
}
