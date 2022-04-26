#include "include/MainWrapper.h"
#include "CLI/App.hpp"
#include "CLI/Formatter.hpp"
#include "CLI/Config.hpp"

int main(int argc, char **argv) {
  // ./Yolo-Fastestv2 -i ../test.jpg -p ../model/yolo-fastestv2-opt.param -b ../model/yolo-fastestv2-opt.bin
  CLI::App app{"A example YoloApp Fastest v2 application"};
  std::string inputFilePath;
  std::string outputFileName;
  std::string paramPath;
  std::string binPath;
  std::string rtmpUrl;
  std::string redisUrl = "tcp://127.0.0.1:6379";
  float scaledCoeffs = 1.0;
  float thresholdNMS = 0.1;
  float outFps = 0.0;
  float cropCoeffs = 0.1;
  int threadsNum = 4;
  bool isDebug = false;
  bool isRedis = true;
  app.add_option("-i,--input", inputFilePath, "Input file location")->required();
  app.add_option("-o,--output", outputFileName, "Output file location");
  app.add_option("-s,--scale", scaledCoeffs, "Scale coefficient for video output")->check(CLI::Range(0.0, 1.0));
  app.add_option("--crop", cropCoeffs, "The rectangle box that recognise the door of elevator")->check(
      CLI::Range(0.0, 0.4));
  app.add_option("--out-fps", outFps, "Manually set output fps")->check(CLI::Range(0.0, 60.0));
  app.add_option("--rtmp", rtmpUrl, "The url of RTMP server. started with 'rtmp://'");
  app.add_option("--redis", redisUrl, "The url of Redis server. started with 'tcp://'");
  app.add_option("--nms", thresholdNMS, "NMS threshold for video output")->check(CLI::Range(0.0, 1.0));
  // I don't think there is anyone running this application on more than 16 thread
  app.add_option("-j", threadsNum, "Threads number")->check(CLI::Range(1, 16));
  app.add_option("-p,--param", paramPath, "ncnn network prototype file (end with .param)")->required()->check(
      CLI::ExistingFile);
  app.add_option("-b,--bin", binPath, "ncnn network model file (end with .bin)")->required()->check(
      CLI::ExistingFile);
  app.add_flag("-d,--debug", isDebug, "Enable debug log");
  CLI11_PARSE(app, argc, argv)

  // Thank you copilot
  auto opts = YoloApp::Main::Options{
      .inputFilePath = inputFilePath,
      .outputFileName = outputFileName,
      .paramPath = paramPath,
      .binPath = binPath,
      .rtmpUrl = rtmpUrl,
      .redisUrl = redisUrl,
      .scaledCoeffs = scaledCoeffs,
      .thresholdNMS = thresholdNMS,
      .outFps = outFps,
      .cropCoeffs = cropCoeffs,
      .threadsNum = threadsNum,
      .isDebug = isDebug,
  };

  auto m = YoloApp::Main::MainWrapper(opts);
  m.init();
  auto push = m.pushRun();
  push.detach();
  m.blockPullRun();

  return YoloApp::Error::SUCCESS;
}


