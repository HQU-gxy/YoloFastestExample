//
// Created by crosstyan on 2022/2/9.
//

#ifndef YOLO_FASTESTV2_DETECT_H
#define YOLO_FASTESTV2_DETECT_H

#include "yolo-fastestv2.h"
#include <benchmark.h>
#include "spdlog/spdlog.h"

extern bool IS_CAPTURE_ENABLED;

std::vector<TargetBox> detectFrame(cv::Mat &detectImg, cv::Mat &drawImg, YoloFastestV2 &api, const std::vector<const char *> &classNames);

auto detectDoor(cv::Mat &detectImg, cv::Mat &drawImg, cv::Rect cropRect);

// TODO: use struct as option instead of params
int handleVideo(cv::VideoCapture &cap, YoloFastestV2 &api, const std::vector<const char *> &classNames,
                const std::string &outputFileName, const std::string &rtmpUrl, float scaledCoeffs, float outFps);
#endif //YOLO_FASTESTV2_DETECT_H

