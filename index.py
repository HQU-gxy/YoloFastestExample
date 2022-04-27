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


class UDPApp:
    def __init__(self, remote_host, remote_port):
        self.host = remote_host
        self.port = remote_port
        self.s = None
        self.q = asyncio.Queue(32)

    async def on_yolo_async(self, xs):
        if self.s is None: self.s = await asyncio_dgram.connect((host, port))
        for x in xs:
            pts = [x.x1, x.y1, x.x2, x.y2]
            byte_list = bytes.fromhex("70") + bytes(mapcat(lambda x: x.to_bytes(2, 'big'), pts))
            await self.s.send(byte_list)
            await self.q.put(byte_list)

    async def udp_server(self):
        if self.s is None: self.s = await asyncio_dgram.connect((host, port))
        while True:
            if not(self.q.empty()):
                d = await self.q.get()
                print("Sent", d)
            data, remote_addr = await self.s.recv()
            print("From Main", data)


host = "127.0.0.1"
port = 12345

# main event_loop
loop = asyncio.get_event_loop()
# set_on_detect_yolo callback
loop_yolo = asyncio.new_event_loop()
u = UDPApp(host, port)


def on_yolo(xs):
    loop_yolo.run_until_complete(u.on_yolo_async(xs))
    # asyncio.create_task(test_async(xs))


opts = yolo_app.init_options(opts_dict)
main = yolo_app.MainWrapper(opts)
main.init()
main.set_on_detect_yolo(on_yolo)
main.run_push()
main.run_pull()

loop.run_until_complete(u.udp_server())
