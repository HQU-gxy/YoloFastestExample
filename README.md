# Yolo-FastestV2

This project is developed in Linux, with Python 3.10. Has not been tested in Windows.

## Build

### Requirements

- Tested with Python 3.10 and 3.9
- C++17 compatible compilers (tested with gcc10)

Compile [Tencent/ncnn](https://github.com/Tencent/ncnn) and install it to `/usr/local/`. See also [Build for Linux](https://github.com/Tencent/ncnn/wiki/how-to-build#build-for-linux) and [Build for ARM Cortex-A family with cross-compiling](https://github.com/Tencent/ncnn/wiki/how-to-build#build-for-arm-cortex-a-family-with-cross-compiling)

```bash
git clone --recursive https://github.com/crosstyan/Yolo_Fastest_Example
```

```bash
# https://pkgs.org/
sudo apt install libspdlog-dev
# https://docs.opencv.org/4.x/d7/d9f/tutorial_linux_install.html
sudo apt install libopencv-dev
sudo apt install libhiredis-dev
sudo apt install python3-gevent
# https://github.com/sewenew/redis-plus-plus/
cd lib/redis-plus-plus
mkdir build
cd build
cmake -DREDIS_PLUS_PLUS_CXX_STANDARD=17 ..
make
make install

sudo apt install libpython3.9-dev
pip install python-config
python-config --includes
# change Python3_INCLUDE_DIR and Python3_LIBRARY

# return to ${PROJECT_SOURCE_DIR}
cd ../../

mkdir build
cd build
# you need to specify the path to Python and OpenCV if needed
# cmake .. -DOpenCV_DIR=/opt/rk3328_cross_compile_opencv
cmake ..
cmake --build . --target yolo_app -- -j $(nproc)
# default install to ${PROJECT_SOURCE_DIR}/py
make
make install
cd ../py
pip install requirements.txt
```

now run `python3 g.py` in `${PROJECT_SOURCE_DIR}/py`

## Build Note

### Swapfile

You need a swapfile to compile it if your memory is not enough. At least 3 GB of memory is needed.

[Create a Linux Swap File](https://linuxize.com/post/create-a-linux-swap-file/)

```bash
sudo free -h
# or
sudo swapon --show
```

```bash
sudo fallocate -l 2G /swapfile
sudo chmod 600 /swapfile
sudo mkswap /swapfile
sudo swapon /swapfile
```

then edit `/etc/fstab`

```text
/swapfile swap swap defaults 0 0
```

Maybe you need to remove the swap file later for small disk space device.

```bash
sudo swapoff -v /swapfile
```

### redis

```bash
apt install redis
```

[Disable Redis Persistent Store to prevent run out of disk](https://stackoverflow.com/questions/28785383/how-to-disable-persistence-with-redis)

Modify `/etc/redis/redis.conf` then restart the redis `systemctl restart redis`

### proxychains

Clash or other proxy software should be accessible via LAN or running locally.

`/etc/proxychains.conf`

### Install GCC10

- [How to install g++ 10 on Ubuntu 18.04?](https://askubuntu.com/questions/1192955/how-to-install-g-10-on-ubuntu-18-04)
- [How to specify new GCC path for CMake](https://stackoverflow.com/questions/17275348/how-to-specify-new-gcc-path-for-cmake)

### Install Python3.10

- [deadsnakes软件源介绍以及在Ubuntu上安装最近版本Python的方法](https://zhuanlan.zhihu.com/p/45329974)
- [deadsnakes PPA](https://tooling.bennuttall.com/deadsnakes/)

```bash
apt install python3.10-dev python3.10-venv
```

using `pip` [Pip is not working for Python 3.10 on Ubuntu](https://stackoverflow.com/questions/69503329/pip-is-not-working-for-python-3-10-on-ubuntu)

Proxy is usually necessary.

```bash
curl -sS https://bootstrap.pypa.io/get-pip.py | python3.10
```

### update-alternatives

[update-alternatives](https://man7.org/linux/man-pages/man1/update-alternatives.1.html)

```bash
sudo update-alternatives --install /usr/bin/python3 python3 /usr/bin/python3.10 3
```

### Install Latest CMake (3.24.0-rc1)

Download binary from [cmake](https://cmake.org/download/).

Copy to `/usr/` (or `/usr/local/`?)

### i2cdetect

```bash
apt-get install i2c-tools
```

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

[What does a C process status mean in htop?](https://stackoverflow.com/questions/18470215/what-does-a-c-process-status-mean-in-htop)
[htop explained](https://peteris.rocks/blog/htop/#s-interruptible-sleep-waiting-for-an-event-to-complete)

```bash
python3 g.py --host 192.168.123.43 -d
```

GStreamer + [gstreamer-rockchip](https://github.com/crosstyan/gstreamer-rockchip) (fork form [firefly-linux/external/gstreamer-rockchip](https://gitlab.com/firefly-linux/external/gstreamer-rockchip)) + [gstreamer-rockchip-extra
](https://github.com/TinkerBoard/gstreamer-rockchip-extra) and
[rockchip-linux/mpp](https://github.com/rockchip-linux/mpp)

```python
base_pipeline = "appsrc ! " + \
                "videoconvert ! " + \
                "mpph264enc ! " + \
                "h264parse ! " + \
                "flvmux streamable=true ! " + \
                "queue ! " + \
                "rtmpsink sync=true location="
```
