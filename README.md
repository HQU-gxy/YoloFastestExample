# Yolo-FastestV2

This project is developed in Linux, with Python 3.10. Has not been tested in Windows.

## Build

```bash
sudo apt install libspdlog-dev
# https://docs.opencv.org/4.x/d7/d9f/tutorial_linux_install.html
sudo apt install libopencv-dev
sudo apt install libhiredis-dev
# https://github.com/sewenew/redis-plus-plus/
cd lib/redis-plus-plus
mkdir build
cd build
cmake -DREDIS_PLUS_PLUS_CXX_STANDARD=17 ..
make
make install

# return to ${PROJECT_SOURCE_DIR}
cd ../../

mkdir build
cd build
# only build python bind
cmake ..
cmake --build . --target yolo_app -- -j $(nproc)
# default install to ${PROJECT_SOURCE_DIR}/py
make install
cd ../py
pip install requirements.txt
```

now run `python3 g.py` in `${PROJECT_SOURCE_DIR}/py`

## IDE

This is a cmake project. Everything should work if your IDE supports it.

### VSCdoe

See [Configure Hello World](https://code.visualstudio.com/docs/cpp/CMake-linux#_configure-hello-world)
also [c-cpp-properties-schema-reference](https://code.visualstudio.com/docs/cpp/c-cpp-properties-schema-reference)

Add `"configurationProvider": "ms-vscode.cmake-tools"` to your `c_cpp_properties.json`

### CLion

Everything is perfect in CLion.

## Notes

using OpenCV's Gstreamer API

```bash
gst-launch-1.0 -v videotestsrc ! x264enc ! flvmux ! rtmpsink location='rtmp://localhost:1935/live/rfBd56ti2SMtYvSgD5xAV0YU99zampta7Z7S575KLkIZ9PYk'
gst-launch-1.0 videotestsrc is-live=true ! x264enc  pass=5 quantizer=25 speed-preset=6 ! video/x-h264, profile=baseline  ! flvmux ! rtmpsink location='rtmp://localhost:1935/live/rfBd56ti2SMtYvSgD5xAV0YU99zampta7Z7S575KLkIZ9PYk'
gst-inspect-1.0 | grep x264
ffmpeg -re -i demo.flv -c copy -f flv rtmp://localhost:1935/live/rfBd56ti2SMtYvSgD5xAV0YU99zampta7Z7S575KLkIZ9PYk
```
