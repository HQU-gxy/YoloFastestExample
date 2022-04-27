import os
import sys
import time

pwd = os.path.dirname(os.path.realpath(__file__))
print(pwd)
# pwd = os.path.dirname(__file__)
so_root = os.path.join(pwd, 'cmake-build-debug')
print(so_root)
sys.path.insert(0, so_root)
import yolo_app

opts_dict = {
    "input_file_path": "0",
    "output_file_path": "",
    "param_path": os.path.join(pwd, "model", "yolo-fastestv2-opt.param"),
    "bin_path": os.path.join(pwd, "model", "yolo-fastestv2-opt.bin"),
    "rtmp_url": "rtmp://localhost:1935/live/test",
    "redis_url": "tcp://127.0.0.1:6379",
    "scaled_coeffs": 1.0,
    "threshold_NMS": 0.1,
    "out_fps": 5,
    "crop_coeffs": 0.1,
    "threads_num": 4,
    "is_debug": True,
}

opts = yolo_app.init_options(opts_dict)
main = yolo_app.MainWrapper(opts)
main.init()
main.run_push()

while True:
    time.sleep(5)
