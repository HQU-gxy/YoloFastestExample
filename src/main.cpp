#include <csignal>
#include "asio.hpp"
#include "include/detect.h"
#include "include/utils.h"
#include "CLI/App.hpp"
// These include are required
// DON'T REMOVE
#include "CLI/Formatter.hpp"
#include "CLI/Config.hpp"


int main(int argc, char **argv) {
  static const std::vector<char const *> classNames = {
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

  // ./Yolo-Fastestv2 -i ../test.jpg -p ../model/yolo-fastestv2-opt.param -b ../model/yolo-fastestv2-opt.bin
  CLI::App app{"A example YOLO Fastest v2 application"};
  std::string inputFilePath;
  std::string outputFileName;
  std::string paramPath;
  std::string binPath;
  std::string rtmpUrl;
  float scaledCoeffs = 1.0;
  float thresholdNMS = 0.1;
  float outFps = 0.0;
  int threadsNum = 4;
  bool isDebug = false;
  app.add_option("-i,--input", inputFilePath, "Input file location")->required();
  app.add_option("-o,--output", outputFileName, "Output file location");
  app.add_option("-s,--scale", scaledCoeffs, "Scale coefficient for video output")->check(CLI::Range(0.0, 1.0));
  app.add_option("--out-fps", outFps, "Manually set output fps")->check(CLI::Range(0.0, 60.0));
  app.add_option("--rtmp", rtmpUrl, "The url of RTMP server. started with 'rtmp://'")->required();
  app.add_option("--nms", thresholdNMS, "NMS threshold for video output")->check(CLI::Range(0.0, 1.0));
  // I don't think there is anyone running this application on more than 16 thread
  app.add_option("-j", threadsNum, "Threads number")->check(CLI::Range(1, 16));
  app.add_option("-p,--param", paramPath, "ncnn network prototype file (end with .param)")->required()->check(
      CLI::ExistingFile);
  app.add_option("-b,--bin", binPath, "ncnn network model file (end with .bin)")->required()->check(
      CLI::ExistingFile);
  app.add_flag("-d,--debug", isDebug, "Enable debug log");
  CLI11_PARSE(app, argc, argv)

  if (isDebug) {
    spdlog::set_level(spdlog::level::debug);
  }

  auto fileType = getFileType(inputFilePath);

  YoloFastestV2 api(threadsNum, thresholdNMS);
  api.loadModel(paramPath.c_str(), binPath.c_str());

  // Ctrl + C
  // Use Signal Sign to tell the application to stop
  signal(SIGINT, [](int sig) {
    spdlog::error("SIGINT is received. Force stopping the application");
    exit(1);
  });

  // Ctrl + Z
  // Don't just use exit() or OpenCV won't save the video correctly
  signal(SIGTSTP, [](int sig) {
    spdlog::warn("SIGTSTP is received. Stopping capture");
    YOLO::IS_CAPTURE_ENABLED = false;
  });

  switch (fileType) {
    case FileType::Image: {
      spdlog::info("Input file is image");
      if (outputFileName.empty()) {
        outputFileName = getOutputFileName(inputFilePath);
      }
      cv::Mat cvImg = cv::imread(inputFilePath);
      auto boxes = detectFrame(cvImg, cvImg,api, classNames);
      if (isDebug) {
        // spdlog::debug("{}\t{}\t{}\t{}\t{}\t{}", "x1", "y1", "x2", "y2", "score", "class");
        for (TargetBox box: boxes) {
          spdlog::debug("{}\t{}\t{}\t{}\t{}\t{}", box.x1, box.y1, box.x2, box.y2, box.score, classNames[box.cate]);
        }
      }
      cv::imwrite(outputFileName, cvImg);
      break;
    }
    case FileType::Video: {
      spdlog::info("Input file is video");
      if (outputFileName.empty()) {
        outputFileName = getOutputFileName(inputFilePath);
      }
      cv::VideoCapture cap(inputFilePath);
      if (!cap.isOpened()) {
        spdlog::error("Cannot open video file");
        return -1;
      }
      handleVideo(cap, api, classNames, outputFileName, rtmpUrl, scaledCoeffs, outFps);
      break;
    }
    case FileType::Stream: {
      auto index = std::stoi(inputFilePath);
      spdlog::info("Streaming from camera {}", index);
      cv::VideoCapture cap(index);
      if (!cap.isOpened()) {
        spdlog::error("Cannot open video file");
        return -1;
      }
      if (outputFileName.empty()) {
        outputFileName = std::to_string(index) + "-out.mp4";
      }
      handleVideo(cap, api, classNames, outputFileName, rtmpUrl, scaledCoeffs, outFps);
      break;
    }
    case (FileType::Unknown): {
      spdlog::error("Unsupported file type");
      return -1;
    }
  }
  return 0;
}


