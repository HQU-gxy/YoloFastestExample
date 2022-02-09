//
// Created by crosstyan on 2022/2/9.
//

#ifndef YOLO_FASTESTV2_UTILS_H
#define YOLO_FASTESTV2_UTILS_H

#include "spdlog/spdlog.h"
#include <filesystem>
#include <opencv2/opencv.hpp>

enum FileType {
  Image,
  Video,
  Stream,
  Unknown
};

FileType getFileType(const std::string &fileName);
int getCodec(const std::string &codec);
std::string getOutputFileName(const std::string &inputFileName, const std::string postFix = "-out");

#endif //YOLO_FASTESTV2_UTILS_H


