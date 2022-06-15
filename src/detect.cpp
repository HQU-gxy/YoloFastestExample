//
// Created by crosstyan on 2022/2/9.
//


#ifndef _STANDALONE_ON

#include "include/detect.h"

namespace py = pybind11;
#endif

using namespace YoloApp;

/// not a pure function, will modify the drawImg
/// @param classNames  the name of the class to be detected (array of strings)
std::vector<TargetBox>
YoloApp::detectFrame(cv::Mat &detectImg,
                     cv::Mat &drawImg,
                     YoloFastestV2 &api,
                     const std::vector<const char *> &classNames,
                     const std::function<void(const std::vector<TargetBox> &)> &cb) {
  std::vector<TargetBox> boxes;

  api.detection(detectImg, boxes);

  for (TargetBox box: boxes) {
    char text[256];
    sprintf(text, "%s %.1f%%", classNames[box.cate], box.score * 100);

    int baseLine = 0;
    cv::Size label_size = cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);

    int x = box.x1;
    int y = box.y1 - label_size.height - baseLine;
    if (y < 0) {
      y = 0;
    }
    if (x + label_size.width > detectImg.cols) {
      x = detectImg.cols - label_size.width;
    }

    cv::rectangle(drawImg, cv::Rect(cv::Point(x, y), cv::Size(label_size.width, label_size.height + baseLine)),
                  cv::Scalar(255, 255, 255), -1);

    cv::putText(drawImg, text, cv::Point(x, y + label_size.height),
                cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));

    cv::rectangle(drawImg, cv::Point(box.x1, box.y1),
                  cv::Point(box.x2, box.y2), cv::Scalar(255, 255, 0), 2, 2, 0);
  }
  // not sure if this is necessary
  #ifndef _STANDALONE_ON
  py::gil_scoped_acquire acquire;
  #endif
  cb(boxes);
  return boxes;
}

auto drawTime(cv::Mat &drawImg, float font_scale = 0.5, int text_x = 10, int text_y = 20) {
  // add current time
  // https://github.com/HowardHinnant/date/issues/543
  auto now = std::chrono::system_clock::now();
  auto formatted = date::format("%Y/%m/%d %H:%M:%S", date::floor<std::chrono::seconds>(now));
  cv::putText(drawImg, formatted, cv::Point(text_x, text_y), cv::FONT_HERSHEY_SIMPLEX, font_scale, cv::Scalar(0, 0, 0),
              2,
              cv::LINE_AA);
  cv::putText(drawImg, formatted, cv::Point(text_x, text_y), cv::FONT_HERSHEY_SIMPLEX, font_scale,
              cv::Scalar(255, 255, 255), 1,
              cv::LINE_AA);
}

auto YoloApp::detectDoor(cv::Mat &detectImg,
                         cv::Mat &drawImg,
                         cv::Rect cropRect,
                         const std::function<void(const std::vector<pt_pair> &)> &cb) {
  cv::Mat processedImg;
  cv::Mat edges;
  cv::Mat lines;

  using namespace std;
  // two pairs of points in a std::pair
  vector<pt_pair> door_lines;
  cv::GaussianBlur(detectImg, processedImg, cv::Size(5, 5), 0, 0);
  cv::Canny(processedImg, edges, 100, 200);
  cv::HoughLinesP(edges, lines, 0.5, CV_PI / 180, 30, 50, 10);
  if (!lines.empty()) {
    auto newDrawImg = drawImg(cropRect);
    // use iterator instead of forEach
    // The big lambda function might cause some problems
    // See also
    // https://docs.opencv.org/4.x/d3/d63/classcv_1_1Mat.html#a952ef1a85d70a510240cb645a90efc0d
    for (auto &line: cv::Mat_<cv::Vec4i>(lines)) {
      auto x1 = line[0];
      auto y1 = line[1];
      auto x2 = line[2];
      auto y2 = line[3];
      auto angle = abs(atan2(y2 - y1, x2 - x1) * 180 / CV_PI);
      if (angle > 85 && angle < 95) {
        cv::line(newDrawImg, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(255, 0, 0), 2, cv::LINE_AA);
        door_lines.emplace_back(make_pair(x1, y1), make_pair(x2, y2));
      } else {
        cv::line(newDrawImg, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 0, 255), 2, cv::LINE_AA);
      }
    }
  }
  #ifndef _STANDALONE_ON
  py::gil_scoped_acquire acquire;
  #endif
  cb(door_lines);
  return door_lines;
}

YoloApp::CapProps YoloApp::getCapProps(cv::VideoCapture &cap) {
  return {
      cap.get(cv::CAP_PROP_FRAME_WIDTH),
      cap.get(cv::CAP_PROP_FRAME_HEIGHT),
      cap.get(cv::CAP_PROP_FPS),
  };
}

YoloApp::CapProps VideoHandler::getCapProps() {
  return {
      1.0 * frame_width, // a trick converting int to double
      1.0 * frame_height,
      cap.get(cv::CAP_PROP_FPS),
  };
}

// I should move the ownership of cap and YoloFastestV2 API to VideoHandler
VideoHandler::VideoHandler(cv::VideoCapture &cap, YoloFastestV2 &api, sw::redis::Redis &redis,
                           const std::vector<const char *> classNames, Options &opts)
    : cap{cap}, api{api}, classNames{classNames}, redis{redis}, opts{opts} {

  frame_width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
  frame_height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
  frame_fps = cap.get(cv::CAP_PROP_FPS);
  frame_count = cap.get(cv::CAP_PROP_FRAME_COUNT);

  spdlog::debug("Default video size: {}x{}", frame_width, frame_height);
  spdlog::debug("Default video fps: {}", frame_fps);
  spdlog::debug("Video frame count: {}", frame_count);

  // convention: targetInputWidth less than zero, then use default parameter.
  if (opts.targetInputWidth > 0 && opts.targetInputHeight > 0) {
    cap.set(cv::CAP_PROP_FRAME_WIDTH, opts.targetInputWidth);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, opts.targetInputHeight);
    frame_width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    frame_height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    spdlog::debug("Try to set video to size: {}x{}, result {}x{}",
                  opts.targetInputWidth, opts.targetInputHeight,
                  frame_width, frame_height);
  }

  if (opts.targetInputFPS > 0) {
    cap.set(cv::CAP_PROP_FPS, opts.targetInputFPS);
    frame_fps = cap.get(cv::CAP_PROP_FPS);
    spdlog::debug("Try to set video FPS to {}, result {}", opts.targetInputFPS, frame_fps);
  }

  redis.del(opts.cacheKey);


  if (opts.scaledCoeffs < 0 || opts.scaledCoeffs > 1) {
    spdlog::warn("scaledCoeffs should be between 0 and 1. Image will not be resized");
  } else {
    frame_width = frame_width * opts.scaledCoeffs;
    frame_height = frame_height * opts.scaledCoeffs;
    spdlog::debug("Final video size: {}x{} with scaledCoeffs {}", frame_width, frame_height, opts.scaledCoeffs);
  }

  // set default rectangle
  const auto CROP_COEF = 0.1;
  const auto vertical_middle = frame_width / 2;
  const auto crop_right_pt = vertical_middle + (frame_width * CROP_COEF);
  const auto crop_left_pt = vertical_middle - (frame_width * CROP_COEF);
  const auto length = crop_right_pt - crop_left_pt;
  this->cropRect = cv::Rect(crop_left_pt, 0, length, length);
}

void VideoHandler::saveToRedis(cv::Mat image, std::string key) {
  // uchar = unsigned char
  std::vector<uchar> buf;
  auto success = cv::imencode(".png", image, buf);

  // spdlog::debug("Send Vector Length: {}", buf.size());
  if (success) {
    auto len = redis.llen(opts.cacheKey);
    // 1500 is the max length of the queue
    // TODO I shouldn't use magic number
    if (len < 1500) {
      // spdlog::debug("Redis List length: {}", len);
      // See https://stackoverflow.com/questions/62363934/how-can-i-store-binary-data-using-redis-plus-plus-like-i-want-to-store-a-structu
      redis.lpush(key, sw::redis::StringView(reinterpret_cast<const char *>(buf.data()), buf.size()));
    } else {
      redis.rpop(key);
      redis.lpush(key, sw::redis::StringView(reinterpret_cast<const char *>(buf.data()), buf.size()));
    }
  } else {
    spdlog::error("Failed to encode image");
    throw std::runtime_error("Failed to encode image");
  }
}

int VideoHandler::run() {
  int real_frame_count = 0;
  while (YoloApp::IS_CAPTURE_ENABLED) {
    cv::Mat cvImg;
    cv::Mat cvImgResized;
    cap >> cvImg;
    if (cvImg.empty()) {
      break;
    }
    auto start = ncnn::get_current_time();
    if (opts.scaledCoeffs < 0 || opts.scaledCoeffs > 1) {
      cvImgResized = std::move(cvImg);
    } else {
      cv::resize(cvImg, cvImgResized, cv::Size(frame_width, frame_height));
    }

    auto origImg = cvImgResized.clone();
    auto croppedImg = cvImgResized(cropRect);

    detectDoor(croppedImg, cvImgResized, cropRect, onDetectDoor);
    if (opts.isBorder) {
      cv::rectangle(cvImgResized, cropRect, cv::Scalar(0, 204, 255), 1, cv::LINE_AA);
    }
    // boxes should not be used
    if (this->isYolo) {
      detectFrame(origImg, cvImgResized, api, classNames, onDetectYolo);
    }
    drawTime(cvImgResized);

    if (opts.isSaveAlt) {
      this->saveToRedis(origImg, opts.altCacheKey);
    }
    this->saveToRedis(cvImgResized, opts.cacheKey);
    auto end = ncnn::get_current_time();

    // Debug info
    if (opts.isDebug) {
      real_frame_count++;
      if (real_frame_count > (INT_MAX - 10)) {
        spdlog::warn("Frame count will be exceeded. Reset to 0");
        real_frame_count = 0;
      }
      if (frame_count > 0) {
        spdlog::debug("[{}/{}]\t{} ms", real_frame_count, frame_count, end - start);
      } else {
        spdlog::debug("[{}]\t{} ms\tYolo:{}", real_frame_count, end - start, this->isYolo);
      }
    }
  }
  return YoloApp::Error::SUCCESS;
}

void VideoHandler::setOnDetectDoor(const std::function<void(const std::vector<pt_pair> &)> &onDetectDoor) {
  VideoHandler::onDetectDoor = onDetectDoor;
}

void VideoHandler::setOnDetectYolo(const std::function<void(const std::vector<TargetBox> &)> &onDetectYolo) {
  VideoHandler::onDetectYolo = onDetectYolo;
}

int VideoHandler::setCropRect(int x, int y, int w, int h) {
  if (x < 0 || y < 0 || w <= 0 || h <= 0) {
    spdlog::error("Input should be positive");
    return YoloApp::Error::FAILURE;
  }
  auto image_rect = cv::Rect(0, 0, frame_width, frame_height);
  auto rect = cv::Rect(x, y, w, h);
  // See https://stackoverflow.com/questions/29120231/how-to-verify-if-rect-is-inside-cvmat-in-opencv
  bool is_valid = (rect & image_rect) == rect;
  if (!is_valid) {
    spdlog::error("Invalid crop rect. Out of image boundary.");
    return YoloApp::Error::FAILURE;
  } else {
    this->cropRect = rect;
    return YoloApp::Error::SUCCESS;
  }
}
