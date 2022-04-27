import os
import sys
import time

# pwd = os.path.dirname(__file__)
pwd = os.path.dirname(os.path.realpath(__file__))
print(pwd)
so_root = os.path.join(pwd, 'cmake-build-debug')
print(so_root)
sys.path.insert(0, so_root)
import yolo_app

base_rtmp_url = "rtmp://localhost:1935/live/"

base_pipeline = "appsrc ! " + \
                "videoconvert ! " + \
                "x264enc  pass=5 quantizer=25 speed-preset=6 ! " + \
                "video/x-h264, profile=baseline ! " + \
                "flvmux ! " + \
                "rtmpsink location="

#  -i 0 -p ../model/yolo-fastestv2-opt.param -b ../model/yolo-fastestv2-opt.bin -d -s 0.2 --nms 0.125 --rtmp rtmp://localhost:1935/live/test
opts_dict = {
    "input_file_path": "0",
    "output_file_path": "",
    "param_path": os.path.join(pwd, "model", "yolo-fastestv2-opt.param"),
    "bin_path": os.path.join(pwd, "model", "yolo-fastestv2-opt.bin"),
    "rtmp_url": base_rtmp_url + "test",
    "redis_url": "tcp://127.0.0.1:6379",
    "scaled_coeffs": 0.2,
    "threshold_NMS": 0.125,
    "out_fps": 5,
    "crop_coeffs": 0.1,
    "threads_num": 4,
    "is_debug": False,
}

def printTarget(xs):
    for x in xs:
        print('From Python: {} {} {} {}'.format(x.x1, x.y1, x.x2, x.y2))

opts = yolo_app.init_options(opts_dict)
main = yolo_app.MainWrapper(opts)
main.init()
main.set_on_detect_yolo(printTarget)
main.run_push()
main.run_pull()

time.sleep(10)

main.swap_pull_writer(base_pipeline + base_rtmp_url + "test2")

while True:
    time.sleep(5)
