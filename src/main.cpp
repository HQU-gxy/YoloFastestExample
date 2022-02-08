#include <benchmark.h>
#include "yolo-fastestv2.h"
#include "CLI/App.hpp"
// These include are required
// DON'T REMOVE
#include "CLI/Formatter.hpp"
#include "CLI/Config.hpp"
#include "spdlog/spdlog.h"

// not a pure function, will modify the cvImg
// @param className - the name of the class to be detected (array of strings)
std::vector<TargetBox> detectFrame(cv::Mat& cvImg, yoloFastestv2& api, const std::vector<char const *> className){
  std::vector<TargetBox> boxes;

  auto start = ncnn::get_current_time();
  api.detection(cvImg, boxes);
  auto end = ncnn::get_current_time();
  auto time = end - start;
  spdlog::info("detection time: {} ms", time);
  cv::putText(cvImg, std::to_string(time) + " ms", cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 2);

  for (TargetBox box: boxes) {
    char text[256];
    sprintf(text, "%s %.1f%%", className[box.cate], box.score * 100);

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

int main(int argc, char **argv) {
  static const std::vector<char const *> class_names = {
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
  std::string inputFileName = "";
  std::string outputFilename = "output.png";
  std::string paramPath = "";
  std::string binPath = "";
  bool isDebug = false;
  app.add_option("-i,--input", inputFileName, "Input file location")->required()->check(
      CLI::ExistingFile);
  app.add_option("-o,--output", outputFilename, "Output file location");
  app.add_option("-p,--param", paramPath, "ncnn network prototype file (end with .param)")->required()->check(CLI::ExistingFile);
  app.add_option("-b,--bin", binPath, "ncnn network model file (end with .bin)")->required()->check(CLI::ExistingFile);
  app.add_flag("-d,--debug", isDebug, "Enable debug log");

  CLI11_PARSE(app, argc, argv);
  if (isDebug) {
    spdlog::set_level(spdlog::level::debug);
  }

  yoloFastestv2 api;
  api.loadModel(paramPath.c_str(), binPath.c_str());

  cv::Mat cvImg = cv::imread(inputFileName);
  auto boxes = detectFrame(cvImg, api, class_names);
  if(isDebug) {
//    spdlog::debug("{}\t{}\t{}\t{}\t{}\t{}", "x1", "y1", "x2", "y2", "score", "class");
    for (TargetBox box : boxes) {
      spdlog::debug("{}\t{}\t{}\t{}\t{}\t{}", box.x1, box.y1, box.x2, box.y2, box.score, class_names[box.cate]);
    }
  }
  cv::imwrite(outputFilename, cvImg);

  return 0;
}


