//
// Created by crosstyan on 2022/2/9.
//

#ifndef YOLO_FASTESTV2_UTILS_H
#define YOLO_FASTESTV2_UTILS_H

#include "spdlog/spdlog.h"
#include <filesystem>
#include <opencv2/opencv.hpp>

namespace YoloApp {
  enum class FileType {
    Image,
    Video,
    Stream,
    Unknown
  };

  FileType getFileType(const std::string &fileName) noexcept;
  int getCodec(const std::string &codec) noexcept;
  std::string getOutputFileName(const std::string &inputFileName, const std::string postFix = "-out") noexcept;
}

#endif //YOLO_FASTESTV2_UTILS_H


