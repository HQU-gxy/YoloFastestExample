#include "yolo-fastestv2.h"
#include "CLI/App.hpp"
#include "CLI/Formatter.hpp"
#include "CLI/Config.hpp"

int main(int argc, char** argv) {
  static const char *class_names[] = {
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
  app.add_option("-i,--input", inputFileName, "Input file location, with extension name of jpg")->required()->check(CLI::ExistingFile);
  app.add_option("-o,--output", outputFilename, "Output file location, with extension name of png");
  app.add_option("-p,--param", paramPath, "ncnn network parameters file")->required()->check(CLI::ExistingFile);
  app.add_option("-b,--bin", binPath, "ncnn network bin file")->required()->check(CLI::ExistingFile);

  CLI11_PARSE(app, argc, argv);

  yoloFastestv2 api;

  api.loadModel(paramPath.c_str(), binPath.c_str());

  cv::Mat cvImg = cv::imread(inputFileName);

  std::vector<TargetBox> boxes;
  api.detection(cvImg, boxes);

  for (TargetBox box : boxes) {
    // print out the boxes
    std::cout << box.x1 << " " << box.y1 << " " << box.x2 << " " << box.y2
              << " " << box.score << " " << class_names[box.cate] << std::endl;

    char text[256];
    sprintf(text, "%s %.1f%%", class_names[box.cate], box.score * 100);

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

  cv::imwrite(outputFilename, cvImg);

  return 0;
}
