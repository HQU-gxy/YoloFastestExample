import os
import sys
import time
import asyncio
import asyncio_dgram
from toolz import mapcat

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

# stream
# make sure server and client are share the same port
global s
s = None

host = "127.0.0.1"
port = 12345

# main event_loop
loop = asyncio.get_event_loop()
# set_on_detect_yolo callback
loop_yolo = asyncio.new_event_loop()


async def test_async(xs):
    global s
    if s is None: s = await asyncio_dgram.connect((host, port))
    for x in xs:
        pts = [x.x1, x.y1, x.x2, x.y2]
        byte_list = bytes.fromhex("70") + bytes(mapcat(lambda x: x.to_bytes(2, 'big'), pts))
        # print(byte_list)
        # print('From Python: {} {} {} {}'.format(x.x1, x.y1, x.x2, x.y2))
        await s.send(byte_list)
        data, remote_addr = await s.recv()
        print("From Yolo", data)


async def udp_server():
    global s
    if s is None: s = await asyncio_dgram.connect((host, port))
    while True:
        data, remote_addr = await s.recv()
        print("From Main", data)


def test(xs):
    loop_yolo.run_until_complete(test_async(xs))
    # asyncio.create_task(test_async(xs))


opts = yolo_app.init_options(opts_dict)
main = yolo_app.MainWrapper(opts)
main.init()
main.set_on_detect_yolo(test)
main.run_push()
main.run_pull()

loop.run_until_complete(udp_server())
