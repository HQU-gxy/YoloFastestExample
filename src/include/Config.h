//
// Created by crosstyan on 2022/6/15.
//

#ifndef YOLO_APP_CONFIG_H
#define YOLO_APP_CONFIG_H

#include <vector>
#include <memory>
#include <string>
#ifndef _STANDALONE_ON
#include <pybind11/pybind11.h>
namespace py = pybind11;
#endif


namespace YoloApp {

  enum Error {
    SUCCESS = 0,
    FAILURE = 1
  };

  extern bool IS_CAPTURE_ENABLED;
  extern const std::vector<char const *> classNames;

// This shit should be global variable/singleton. Maybe.
  struct Options {
    std::string inputFilePath;
    std::string paramPath;
    std::string binPath;
    std::string redisUrl = "tcp://127.0.0.1:6379";
    float scaledCoeffs = 1.0;
    int targetInputWidth = -1; // target means MAYBE the value will be set
    int targetInputHeight = -1; // maybe not
    float thresholdNMS = 0.15;
    float targetInputFPS = -1;
    // Should be set if input is Web Camera
    // but not if input is video file (set the value to a negative number)
    float outputFPS = 5;
    int threadsNum = 1;
    bool isBorder = true; // if true, display elevator door detect border
    bool isDebug = false;
  public:
    static std::shared_ptr<Options> get(){
      static auto INSTANCE = std::make_shared<Options>();
      return INSTANCE;
    }
    #ifndef _STANDALONE_ON
    static std::shared_ptr<Options> fromPyDict(const py::dict &dict);
    #endif

    Options () = default;
    // copy constructor should not be used
    Options (Options const&) = delete;
    // the same as move constructor
    Options (Options const&&) = delete;
    Options& operator=(Options const&) = delete;
    Options& operator=(Options &&) = delete;
  };
};


#endif //YOLO_APP_CONFIG_H
