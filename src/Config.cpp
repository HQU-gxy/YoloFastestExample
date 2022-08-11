//
// Created by crosstyan on 2022/6/15.
//

#include "include/Config.h"

/// a global flag in order to make signal function work
bool YoloApp::IS_CAPTURE_ENABLED = true;
const std::vector<char const *> YoloApp::classNames = {
    "person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck", "boat", "traffic light",
    "fire hydrant", "stop sign", "parking meter", "bench", "bird", "cat", "dog", "horse", "sheep", "cow",
    "elephant", "bear", "zebra", "giraffe", "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee",
    "skis", "snowboard", "sports ball", "kite", "baseball bat", "baseball glove", "skateboard", "surfboard",
    "tennis racket", "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl", "banana", "apple",
    "sandwich", "orange", "broccoli", "carrot", "hot dog", "pizza", "donut", "cake", "chair", "couch",
    "potted plant", "bed", "dining table", "toilet", "tv", "laptop", "mouse", "remote", "keyboard", "cell phone",
    "microwave", "oven", "toaster", "sink", "refrigerator", "book", "clock", "vase", "scissors", "teddy bear",
    "hair drier", "toothbrush"
};

using namespace YoloApp;
#ifndef _STANDALONE_ON

std::shared_ptr<Options> Options::fromPyDict(const py::dict &dict) {
  auto instance = Options::get();
  instance->inputFilePath = dict["input_file_path"].cast<std::string>();
  instance->paramPath = dict["param_path"].cast<std::string>();
  instance->binPath = dict["bin_path"].cast<std::string>();
  instance->redisUrl = dict["redis_url"].cast<std::string>();
  instance->scaledCoeffs = dict["scaled_coeffs"].cast<float>();
  instance->thresholdNMS = dict["threshold_NMS"].cast<float>();
  instance->targetInputWidth = dict["target_input_width"].cast<int>();
  instance->targetInputHeight = dict["target_input_height"].cast<int>();
  instance->targetInputFPS = dict["target_input_fps"].cast<float>();
  instance->timeTextX = dict["time_text_x"].cast<int>();
  instance->timeTextY = dict["time_text_y"].cast<int>();
  instance->timeFontScale = dict["time_font_scale"].cast<float>();
  instance->outputFPS = dict["out_fps"].cast<float>();
  instance->threadsNum = dict["threads_num"].cast<int>();
  instance->isBorder = dict["is_border"].cast<bool>();
  instance->isDebug = dict["is_debug"].cast<bool>();
  instance->isDrawTime = dict["is_draw_time"].cast<bool>();
  instance->isSaveAlt = dict["is_save_alt"].cast<bool>();
  instance->cacheKey = dict["cache_key"].cast<std::string>();
  instance->altCacheKey = dict["alt_cache_key"].cast<std::string>();
  return instance;
}

#endif
