#include <benchmark.h>
#include <filesystem>
#include "yolo-fastestv2.h"
#include "CLI/App.hpp"
// These include are required
// DON'T REMOVE
#include "CLI/Formatter.hpp"
#include "CLI/Config.hpp"
#include "spdlog/spdlog.h"

// not a pure function, will modify the cvImg
// @param classNames - the name of the class to be detected (array of strings)
std::vector<TargetBox> detectFrame(cv::Mat &cvImg, YoloFastestV2 &api, const std::vector<char const *> &classNames) {
  std::vector<TargetBox> boxes;

  api.detection(cvImg, boxes);

  for (TargetBox box: boxes) {
    char text[256];
    sprintf(text, "%s %.1f%%", classNames[box.cate], box.score * 100);

    int baseLine = 0;
    cv::Size label_size = cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);

    int x = box.x1;
    int y = box.y1 - label_size.height - baseLine;
    if (y < 0)
      y = 0;
    if (x + label_size.width > cvImg.cols)
      x = cvImg.cols - label_size.width;

    cv::rectangle(cvImg, cv::Rect(cv::Point(x, y), cv::Size(label_size.width, label_size.height + baseLine)),
                  cv::Scalar(255, 255, 255), -1);

    cv::putText(cvImg, text, cv::Point(x, y + label_size.height),
                cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));

    cv::rectangle(cvImg, cv::Point(box.x1, box.y1),
                  cv::Point(box.x2, box.y2), cv::Scalar(255, 255, 0), 2, 2, 0);
  }
  return boxes;
}

enum FileType {
  Image,
  Video,
  Unknown
};

FileType getFileType(const std::string &fileName) {
  std::filesystem::path inputPath(fileName);
  auto inputExtension = inputPath.extension().string();
  spdlog::debug("Input file extension: {}", inputExtension);
  // TODO: consider to use pattern matching
  if (inputExtension == ".jpg" || inputExtension == ".jpeg" || inputExtension == ".png") {
    return FileType::Image;
  } else if (inputExtension == ".mp4" || inputExtension == ".avi" || inputExtension == ".mov" ||
             inputExtension == ".mkv") {
    return FileType::Video;
  } else {
    return FileType::Unknown;
  }
}

int getCodec(const std::string &codec) {
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

std::string getOutputFileName(const std::string &inputFileName, const std::string postFix = "-out") {
  std::filesystem::path inputPath(inputFileName);
  return inputPath.stem().string() + postFix + inputPath.extension().string();
}

// TODO: use struct as option instead of params
int handleVideo(cv::VideoCapture &cap, YoloFastestV2 &api, const std::vector<char const *> &classNames,
                const std::string &outputFileName, int codec, float scaledCoeffs) {

  int frame_width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
  int frame_height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
  int frame_fps = cap.get(cv::CAP_PROP_FPS);
  int frame_count = cap.get(cv::CAP_PROP_FRAME_COUNT);
  int real_frame_count = 0;

  spdlog::debug("Output video codec fourcc: 0x{0:x}", codec);
  cv::VideoWriter outputVideo(outputFileName, codec, frame_fps,
                              cv::Size(frame_width * scaledCoeffs, frame_height * scaledCoeffs));

  spdlog::debug("Original video size: {}x{}", frame_width, frame_height);
  spdlog::debug("Output video size: {}x{}", frame_width * scaledCoeffs, frame_height * scaledCoeffs);
  spdlog::debug("Original video fps: {}", frame_fps);
  spdlog::debug("Original video frame count: {}", frame_count);
  while (true) {
    cv::Mat cvImg;
    cv::Mat cvImgResized;
    cap >> cvImg;
    if (cvImg.empty()) {
      break;
    }
    auto start = ncnn::get_current_time();
    cv::resize(cvImg, cvImgResized, cv::Size(frame_width * scaledCoeffs, frame_height * scaledCoeffs));
    auto boxes = detectFrame(cvImgResized, api, classNames);
    outputVideo.write(cvImgResized);
    auto end = ncnn::get_current_time();

    real_frame_count++;
    if (frame_count > 0) {
      spdlog::info("[{}/{}]\t{} ms", real_frame_count, frame_count, end - start);
    } else {
      spdlog::info("[{}]\t{} ms", real_frame_count, end - start);
    }
  }
  return 0;
}

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
  std::string codec = "mp4v";
  float scaledCoeffs = 1.0;
  float thresholdNMS = 0.1;
  int threadsNum = 4;
  bool isDebug = false;
  app.add_option("-i,--input", inputFilePath, "Input file location")->required()->check(
      CLI::ExistingFile);
  app.add_option("-o,--output", outputFileName, "Output file location");
  app.add_option("-s,--scale", scaledCoeffs, "Scale coefficient for video output")->check(CLI::Range(0.0, 1.0));
  app.add_option("-c,--codec", codec, "Codec for video output");
  app.add_option("--hms", thresholdNMS, "NMS threshold for video output")->check(CLI::Range(0.0, 1.0));
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
  if (outputFileName.empty()) {
    outputFileName = getOutputFileName(inputFilePath);
  }

  YoloFastestV2 api(threadsNum, thresholdNMS);
  api.loadModel(paramPath.c_str(), binPath.c_str());

  switch (fileType) {
    case FileType::Image: {
      spdlog::info("Input file is image");
      cv::Mat cvImg = cv::imread(inputFilePath);
      auto boxes = detectFrame(cvImg, api, classNames);
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
      cv::VideoCapture cap(inputFilePath);
      if (!cap.isOpened()) {
        spdlog::error("Cannot open video file");
        return -1;
      }
      int codecCV = getCodec(codec);
      handleVideo(cap, api, classNames, outputFileName, codecCV, scaledCoeffs);
      break;
    }
    default: {
      spdlog::error("Unsupported file type");
      return -1;
    }
  }
  return 0;
}


