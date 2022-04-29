
# Yolo-FastestV2

This project is developed in Linux, with Python 3.10. Has not been tested in Windows.

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
