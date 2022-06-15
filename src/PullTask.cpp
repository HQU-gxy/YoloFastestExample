//
// Created by crosstyan on 2022/6/15.
//

#include "include/PullTask.h"

using namespace YoloApp;
void PullTask::run() {
  while (YoloApp::IS_CAPTURE_ENABLED) {
    if (this->isReadRedis) {
      //  sw::redis::OptionalStringPair redisMemory = redis.brpop("image", 0);
      /// It's wasteful to copy the data, but it's the only way to get the data
      /// without causing problems.
      /// Also See https://stackoverflow.com/questions/41462433/can-i-reinterpret-stdvectorchar-as-a-stdvectorunsigned-char-without-copy
      if (this->writer != nullptr && !pipeline.empty()) {
        if (!this->writer->isOpened()) {
          auto[frame_width, frame_height, frame_fps] = capProps;
          auto out_framerate = this->opts.outputFPS > 0 ? this->opts.outputFPS : frame_fps;
          spdlog::debug("Opening Video Writer with {}x{} and {} fps", frame_width, frame_height, out_framerate);

          this->writer->open(pipeline, cv::CAP_GSTREAMER, 0, out_framerate,
                             cv::Size(frame_width, frame_height));
        }
      }
      auto redisMemory = this->redis.brpop("image", 0) // TODO: why the key of redis is hardcoded
          .value_or(std::make_pair("", ""))
          .second;
      if (redisMemory.empty()) {
        continue;
      }
      auto vector = std::vector<uchar>(redisMemory.begin(), redisMemory.end());
      auto start = ncnn::get_current_time();
      auto image = cv::imdecode(vector, cv::IMREAD_COLOR);
      if (image.empty()) {
        spdlog::error("Failed to decode image");
        throw std::runtime_error("Failed to decode image");
      }
      if (this->writer == nullptr || !this->writer->isOpened()) {
        spdlog::error("writer is null. Writing will be skipped");
      } else {
        writer->write(image);
      }
      auto end = ncnn::get_current_time();
      spdlog::debug("[Pull/{}]\t{} ms", this->poll, end - start);
      poll++;
      if (poll > this->maxPoll) {
        isReadRedis = false;
        this->onPollComplete(this->poll);
        this->writer->release();
        this->writer = nullptr;
      }
    }
  }
}


void PullTask::clearQueue() {
  this->redis.del("image"); // TODO: why the key of redis is hardcoded
};

PullTask::PullTask(CapProps capProps, Options &opts, sw::redis::Redis &redis) : capProps(capProps), opts(opts),
                                                                                redis(redis) {}

void PullTask::setVideoWriter(std::string pipe) {
  this->pipeline = std::move(pipe);
  if (this->writer != nullptr && this->writer->isOpened()) {
    this->writer->release();
  }
  this->writer = std::make_unique<cv::VideoWriter>();
  // make an empty writer without opening it
  // the opening operation will be finished in PullTask::run()
}
