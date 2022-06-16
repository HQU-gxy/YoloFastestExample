import os
import time
import sys
import gevent
from gevent import socket, Greenlet, sleep
from toolz import mapcat as flatmap
from e_helper import *
from typing import List, TypeVar
# import logging
from loguru import logger
import struct
import argparse

# for so file
pwd = os.path.dirname(os.path.realpath(__file__))
print(pwd)
so_root = os.path.join(pwd)
print(so_root)
sys.path.insert(0, so_root)
import yolo_app

parser = argparse.ArgumentParser(description='Yolo App')
parser.add_argument(
    '--host',
    type=str,
    default='127.0.0.1',
    help='the host of remote server',
    metavar='PARAM',
)

parser.add_argument(
    '--port', '-p',
    type=int,
    default=12345,
    help='the port of remote server',
    metavar='PARAM',
)

parser.add_argument(
    '--debug', '-d',
    action='store_true',
    help='if is debug',
)

default_chan = "unknown"


emerg_max_poll = 60
stream_max_poll = 1500

# base_pipeline = "appsrc ! " + \
#                 "videoconvert ! " + \
#                 "mpph264enc ! " + \
#                 "h264parse ! " + \
#                 "flvmux streamable=true ! " + \
#                 "queue ! " + \
#                 "rtmpsink sync=true location="

base_pipeline = "appsrc ! " + \
                "videoconvert ! " + \
                "x264enc  pass=5 quantizer=25 speed-preset=6 ! " + \
                "video/x-h264, profile=baseline ! " + \
                "flvmux ! " + \
                "rtmpsink sync=false location="


# https://www.gevent.org/examples/udp_client.html
# https://www.gevent.org/examples/udp_server.html

# stream
# make sure server and client are share the same port

T = TypeVar("T")


def drop(coll: list[T], n: int = 1) -> list[T]:
    return coll[n:]


def take(coll: list[T], n: int = 1) -> list[T]:
    return coll[:n]


def bytes_to_uint_str(b: bytes) -> str:
    return str(int.from_bytes(b, byteorder='big', signed=False))


def int_to_bytes(x: int, n=2):
    """big endian"""
    return x.to_bytes(n, byteorder='big')


def gen_pipeline(chan):
    if (chan != None):
        return base_pipeline + base_rtmp_url + chan
    else:
        logger.error("channel is None. Fallback to default")
        return base_pipeline + base_rtmp_url + chan


class UDPApp:
    def __init__(self, remote_host: str, remote_port: int, yolo_app, id: int):
        self.id = id
        self.host = remote_host
        self.port = remote_port
        self.status = Code.OK
        address = (remote_host, remote_port)
        # yolo_app is MainWrapper
        self.hash:   None | int = None
        self.e_chan:  None | str = None
        self.app = yolo_app
        self.sock = socket.socket(type=socket.SOCK_DGRAM)
        self.sock.connect(address)
        # yolo_app initialization
        self.app.init()
        # default pybind11 using unique_ptr
        # so the ownership has been transferred to python side
        # MainWrapper can't access it anymore
        # See 
        # https://pybind11.readthedocs.io/en/stable/advanced/smart_ptrs.html?highlight=shared_ptr#std-shared-ptr
        self.video_handler = self.app.get_handler()
        self.pull_job = self.app.get_alt_pull_job()
        self.pull_job.set_on_poll_complete(self.on_poll_complete)
        self.video_handler.set_on_detect_yolo(self.on_detect_yolo)
        self.video_handler.set_on_detect_door(self.on_detect_door)

    # should call this instead of self.app.reset_poll
    # when reset poll
    def reset_poll(self):
        self.pull_job.reset_poll()
        self.pull_job.max_poll = emerg_max_poll
        self.status = Code.OK.value

    def on_detect_yolo(self, xs):
        if xs:
            for x in xs:
                # pts = [x.x1, x.y1, x.x2, x.y2]
                # byte_list = bytes.fromhex("70") + \
                #     bytes(flatmap(lambda x: x.to_bytes(
                #         2, 'big', signed=True), pts))
                logger.debug("({},{}) ({},{}) category: {} score: {}"
                             .format(x.x1, x.y1, x.x2, x.y2, x.cate, x. score))
                # self.sock.send(byte_list)
                # print("[yolo] send {}".format(byte_list.hex()))

    def on_detect_door(self, xs):
        if xs:
            for x in xs:
                (x1, y1), (x2, y2) = x
                # pts = [x1, y1, x2, y2]
                # byte_list = bytes.fromhex("80") + \
                #     bytes(flatmap(lambda x: x.to_bytes(
                #         2, 'big', signed=True), pts))
                logger.trace("({},{}) ({},{})"
                             .format(x1, y1, x2, y2))
                # self.sock.send(byte_list)
                # print("[door] send {}".format(byte_list.hex()))

    def on_poll_complete(self, poll):
        self.reset_poll()
        logger.debug("poll completes! frame count {}".format(poll))
        self.status = Code.OK.value

    # https://docs.python.org/3/library/struct.html
    # https://www.educative.io/edpresso/what-is-the-python-struct-module
    def handle_req(self, msg: bytes):
        h = list(msg)[0]
        if h == MsgType.INIT.value:
            try:
                hash: int
                _h, hash = struct.unpack(MsgStruct.INIT_SERVER.value, msg)
                self.hash = hash
                logger.info("set hash as {}".format(
                    self.hash.to_bytes(4, 'big').hex()))
            except:
                logger.error("unpack hash error")
                raise Exception("Can't get hash")
        elif h == MsgType.RTMP_EMERG.value:
            hash: int
            chan: int
            _h, hash, chan = struct.unpack(
                MsgStruct.RTMP_EMERG_SERVER.value, msg)
            if (self.hash and self.hash == hash):
                self.e_chan = chan.to_bytes(2, 'big').hex()
                logger.info("set channel as {}".format(self.e_chan))
        elif h == MsgType.RTMP_STREAM.value:
            head: int
            hash: int
            chan: int
            head, hash, chan = struct.unpack(
                MsgStruct.RTMP_STREAM_SERVER.value, msg)
            if (self.hash and self.hash == hash):
                chn_s = chan.to_bytes(2, 'big').hex()
                logger.info("Receive RTMP Channel {}".format(chn_s))
                if (self.pull_job.is_running() == False):
                    self.reset_poll()
                    # self.app.clear_queue()

                    self.pull_job.max_poll = stream_max_poll
                    self.pull_job.start_poll(gen_pipeline(chn_s))

                    logger.info("Start RTMP to {}".format(chn_s))
                    resp = struct.pack(MsgStruct.RTMP_STREAM_CLIENT.value,
                                       head,
                                       hash,
                                       chan,
                                       Code.OK.value)
                    self.sock.send(resp)
                    self.status = Code.BUSY_STREAM
                else:
                    logger.warning("Pull Task is busy")
                    resp = struct.pack(MsgStruct.RTMP_STREAM_CLIENT.value,
                                       head,
                                       hash,
                                       chan,
                                       Code.BUSY.value)
                    self.sock.send(resp)
        elif h == MsgType.RTMP_STOP.value:
            head: int
            hash: int
            head, hash = struct.unpack(
                MsgStruct.RTMP_STOP_SERVER.value, msg)
            if (self.hash and self.hash == hash):
                self.reset_poll()
                resp = struct.pack(MsgStruct.RTMP_STOP_CLIENT.value,
                                   head,
                                   hash,
                                   Code.OK.value)
                self.sock.send(resp)

        else:
            logger.warning("Invalid message {}".format(msg.hex()))

    def send_init_req(self):
        req = struct.pack(MsgStruct.INIT_CLIENT.value,
                          MsgType.INIT.value,
                          self.id)
        self.sock.send(req)
        logger.info("Sent Init {}".format(req.hex()))

    def send_request_e_chan(self):
        req = struct.pack(MsgStruct.RTMP_EMERG_CLIENT.value,
                          MsgType.RTMP_EMERG.value,
                          self.hash)
        self.sock.send(req)
        logger.info("Request new e-chan {}".format(req.hex()))

    def serve_forever(self):
        # buffer size 8192 bytes
        while True:
            data, address = self.sock.recvfrom(1024)
            logger.debug("{} from {}".format(data.hex(), address))
            self.handle_req(data)


def run_main():
    gevent.sleep(1)
    # start threads
    u.app.run_push()
    u.app.run_pull()
    u.app.run_alt_pull()
    # sleep(10)
    # main.reset_poll()
    # main.start_poll(gen_pipeline(u.e_chan))
    # sleep(10)
    # u.send_request_e_chan()
    # sleep(1)
    # main.reset_poll()
    # main.start_poll(gen_pipeline(u.e_chan))


if __name__ == "__main__":
    args = parser.parse_args()
    host = args.host
    port = args.port
    is_debug = args.debug
    base_rtmp_url = "rtmp://{}:1935/live/".format(host)
    opts_dict = {
        "input_file_path": "0",
        # "input_file_path": os.path.join(pwd, "test.mp4"),
        # "input_file_path": "/home/crosstyan/Code/ncnn/py/test (115).mp4",
        "param_path": os.path.join(pwd, "..", "model", "yolo-fastestv2-opt.param"),
        "bin_path": os.path.join(pwd, "..", "model", "yolo-fastestv2-opt.bin"),
        "rtmp_url": base_rtmp_url + default_chan,
        "redis_url": "tcp://127.0.0.1:6379",
        "cache_key": "image1",
        "alt_cache_key": "image2",
        "target_input_width": 640,
        "target_input_height": 480,
        "target_input_fps": -1,
        "scaled_coeffs": 0.2,
        "threshold_NMS": 0.125,
        "out_fps": 15,
        "threads_num": 4,
        "time_text_x": 10,
        "time_text_y": 20,
        "time_font_scale": 0.4,
        "is_border": False,
        "is_draw_time": True,
        "is_save_alt": True,
        "is_debug": is_debug,
    }

    opts = yolo_app.Options.init(opts_dict)
    # https://loguru.readthedocs.io/en/stable/resources/recipes.html#changing-the-level-of-an-existing-handler
    # https://github.com/Delgan/loguru/issues/138
    loguru_level = "DEBUG" if is_debug else "INFO"
    logger.remove()
    logger.add(sys.stderr, level=loguru_level)

    main = yolo_app.MainWrapper(opts)
    logger.info("host: {}, port: {}".format(host, port))
    u = UDPApp(host, port, main, 123)
    # https://sdiehl.github.io/gevent-tutorial/
    g = gevent.spawn(u.serve_forever)
    send_init = gevent.spawn(u.send_init_req)
    run_g = gevent.spawn(run_main)
    g.start()
    # gevent.joinall([g, send_init, run_g])
    gevent.joinall([g, send_init, run_g])
