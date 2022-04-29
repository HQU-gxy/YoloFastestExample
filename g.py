import os
import time
import sys
from gevent import socket
from toolz import mapcat as flatmap
from e_helper import *

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

opts_dict = {
    "input_file_path": "0",
    # "input_file_path": os.path.join(so_root, "test.mp4"),
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
    "is_debug": True,
}

# https://www.gevent.org/examples/udp_client.html
# https://www.gevent.org/examples/udp_server.html

# stream
# make sure server and client are share the same port


class UDPApp:
    def __init__(self, remote_host, remote_port, yolo_app):
        self.host = remote_host
        self.port = remote_port
        address = (remote_host, remote_port)
        # yolo_app is MainWrapper
        self.hash = None
        self.app = yolo_app
        self.sock = socket.socket(type=socket.SOCK_DGRAM)
        self.sock.connect(address)

    def on_detect_yolo(self, xs):
        if xs:
            for x in xs:
                pts = [x.x1, x.y1, x.x2, x.y2]
                byte_list = bytes.fromhex("70") + \
                    bytes(flatmap(lambda x: x.to_bytes(2, 'big', signed=True), pts))
                self.sock.send(byte_list)
                print("[yolo] send {}".format(byte_list.hex()))

    def on_detect_door(self, xs):
        if xs:
            for x in xs:
                (x1, y1), (x2, y2) = x
                pts = [x1, y1, x2, y2]
                byte_list = bytes.fromhex("80") + \
                    bytes(flatmap(lambda x: x.to_bytes(2, 'big', signed=True), pts))
                self.sock.send(byte_list)
                print("[door] send {}".format(byte_list.hex()))

    def serve_forever(self):
        # buffer size
        while True:
            data, address = self.sock.recvfrom(8192)
            print("[main] {} from {}".format(data.hex(), address))


host = "127.0.0.1"
port = 12345

if __name__ == "__main__":
    opts = yolo_app.init_options(opts_dict)
    main = yolo_app.MainWrapper(opts)
    main.init()
    u = UDPApp(host, port, main)
    # main.set_on_detect_yolo(u.on_detect_yolo)
    # main.set_on_detect_door(u.on_detect_door)
    main.set_pull_task_state(True)
    main.run_push()
    main.run_pull()
    # u.serve_forever()
