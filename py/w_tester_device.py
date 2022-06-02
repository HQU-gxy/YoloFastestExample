
import yolo_app
from operator import is_
from subprocess import call
import gevent
from gevent import socket, Greenlet, sleep
from toolz import mapcat as flatmap
from w_helper import *
import struct
import argparse
import datetime
from bmp180 import bmp180
from mpu6050 import mpu6050
from loguru import logger
import os
import time
import sys

pwd = os.path.dirname(os.path.realpath(__file__))
print(pwd)
so_root = os.path.join(pwd)
print(so_root)
sys.path.insert(0, so_root)


def gen_pipeline(chan):
    if (chan != None):
        return base_pipeline + base_rtmp_url + chan
    else:
        logger.error("channel is None. Fallback to default")
        return base_pipeline + base_rtmp_url + chan


emerg_max_poll = 400
stream_max_poll = 1500
base_pipeline = "appsrc ! " + \
                "videoconvert ! " + \
                "x264enc  pass=5 quantizer=25 speed-preset=6 ! " + \
                "video/x-h264, profile=baseline ! " + \
                "flvmux ! " + \
                "rtmpsink location="


def convert_float_to_fixed(num: float):
    return int(num * 100)


class UDPApp:
    def __init__(self, remote_host: str, remote_port: int, yolo_app, id: bytes):
        self.id = id
        self.host = remote_host
        self.port = remote_port
        self.status = Code.OK
        address = (remote_host, remote_port)
        # yolo_app is MainWrapper
        self.key:   None | bytes = None
        self.chan: None | bytes = None
        self.e_chan: None | bytes = None
        self.height = 0
        self.dist = 0
        # self.e_chan:  None | str = None
        self.app = yolo_app
        self.sock = socket.socket(type=socket.SOCK_DGRAM)
        self.acc = mpu6050(0x68)
        self.bmp = bmp180(0x77)
        self.sock.connect(address)
        self.info = {
            "door_open": 0,
            "door_close": 0,
            "people_num": 0,
            "floor": 5,
            "call": 0,
            "x": 110,  # mul 100 (fixed point)
            "y": 240,
            "z": 410,
            "ang_x": 980,
            "ang_y": 280,
            "ang_z": 420,
            "network": 0,
            "signal": 10,
            "error": 0,
            "battery": 38,
            "speed": 180,  # mul 100 (fixed point)
            "pressure": 100560
        }

    def reset_poll(self):
        self.app.reset_poll()
        self.app.set_max_poll(emerg_max_poll)
        self.status = Code.OK.value

    def on_detect_yolo(self, xs):
        if xs:
            for x in xs:
                logger.trace("({},{}) ({},{}) category: {} score: {}"
                             .format(x.x1, x.y1, x.x2, x.y2, x.cate, x. score))

    def on_detect_door(self, xs):
        if xs:
            for x in xs:
                (x1, y1), (x2, y2) = x
                logger.trace("({},{}) ({},{})"
                             .format(x1, y1, x2, y2))

    def on_poll_complete(self, poll):
        self.reset_poll()
        logger.debug("poll completes! frame count {}".format(poll))
        self.status = Code.OK.value

    def read_acc(self):
        accel_data = self.acc.get_accel_data()
        gyro_data = self.acc.get_gyro_data()
        if accel_data != None:
            self.info["x"] = convert_float_to_fixed(accel_data['x'])
            self.info["y"] = convert_float_to_fixed(accel_data['y'])
            self.info["z"] = convert_float_to_fixed(accel_data['z'])
            logger.debug("x: {}, y: {}, z: {}".format(
                self.info["x"], self.info["y"], self.info["z"]))
        if gyro_data != None:
            self.info["ang_x"] = convert_float_to_fixed(gyro_data['x'])
            self.info["ang_y"] = convert_float_to_fixed(gyro_data['y'])
            self.info["ang_z"] = convert_float_to_fixed(gyro_data['z'])
            logger.debug("x: {}, y: {}, z: {}".format(self.info["ang_x"],
                                                      self.info["ang_y"],
                                                      self.info["ang_z"]))

    def read_bmp(self):
        self.info["pressure"] = int(self.bmp.get_pressure())
        logger.debug("bmp: {}".format(self.info["pressure"]))

    def read_sensor_forever(self):
        while True:
            self.read_acc()
            self.read_bmp()
            sleep(1)

    def send_init_req(self):
        req = struct.pack(MsgStruct.INIT_CLIENT.value,
                          MsgType.INIT.value,
                          self.id,
                          Code.END.value)
        self.sock.send(req)
        logger.info("Sent Init {}".format(req.hex()))

    def serve_forever(self):
        # buffer size 8192 bytes
        while True:
            data, address = self.sock.recvfrom(1024)
            logger.debug("{} from {}".format(data.hex(), address))
            self.handle_req(data)

    def send_status_forever(self):
        while True:
            if(self.key != None):
                self.send_status()
            sleep(1)

    # def read_bmp(self):
    def send_status(self):
        req = struct.pack(MsgStruct.STATUS_CLIENT.value,
                          MsgType.STATUS.value,
                          self.key,
                          self.info["door_open"],
                          self.info["door_close"],
                          self.info["people_num"],
                          0, 0, 0, 0, 0,
                          self.info["floor"],
                          self.info["call"],
                          self.info["x"],
                          self.info["y"],
                          self.info["z"],
                          self.info["ang_x"],
                          self.info["ang_y"],
                          self.info["ang_z"],
                          self.info["network"],
                          self.info["signal"],
                          self.info["error"],
                          self.info["battery"],
                          self.info["speed"],
                          self.info["pressure"])
        logger.debug("Send status {}".format(req.hex()))
        self.sock.send(req)

    def handle_req(self, msg: bytes):
        h = list(msg)[0]
        if h == MsgTypeServer.INIT.value:
            try:
                key: bytes
                height: int
                dist: int
                _h, _zero, key, height, dist = struct.unpack(
                    MsgStruct.INIT_SERVER.value, msg)
                self.key = key
                self.height = height
                self.dist = dist
                logger.info("set key as {}. height={}, dist={}".format(
                    key.hex(), height, dist))
            except:
                logger.error("unpack key error")
                raise Exception("Can't get key")

        elif h == MsgTypeServer.STATUS.value:
            # timestamp is unix timestamp
            _head, error, timestamp = struct.unpack(
                MsgStruct.STATUS_SERVER.value, msg)
            unix_timestamp = int.from_bytes(timestamp, byteorder='big')
            dt = datetime.datetime.fromtimestamp(unix_timestamp)
            logger.info("{} {} from {}".format(
                error, dt.strftime('%Y-%m-%d %H:%M:%S'), msg.hex()))
        elif h == MsgTypeServer.RTMP_EMERG.value:
            e_chan: bytes
            _head, error, e_chan = struct.unpack(
                MsgStruct.RTMP_EMERG_SERVER.value, msg)
            self.e_chan = e_chan
        elif h == MsgTypeServer.RTMP_STREAM.value:
            chan: bytes
            _h, chan = struct.unpack(MsgStruct.RTMP_STREAM_SERVER.value, msg)
            chn_s = chan.hex()
            client_header = MsgType.RTMP_STREAM.value
            self.chan = chan
            if (self.app.get_pull_task_state() == False):
                self.reset_poll()
                # self.app.clear_queue()
                self.app.set_max_poll(stream_max_poll)
                self.app.start_poll(gen_pipeline(chn_s))

                logger.info("Start RTMP to {}".format(chn_s))
                resp = struct.pack(MsgStruct.RTMP_STREAM_CLIENT.value,
                                   client_header,
                                   self.key,
                                   Code.OK.value)
                self.sock.send(resp)
            else:
                logger.warning("Pull Task is busy")
                resp = struct.pack(MsgStruct.RTMP_STREAM_CLIENT.value,
                                   client_header,
                                   self.key,
                                   Code.BUSY.value)
                self.sock.send(resp)

        elif h == MsgType.RTMP_STOP.value:
            self.reset_poll()
            client_header = MsgType.RTMP_STOP.value
            resp = struct.pack(MsgStruct.RTMP_STOP_CLIENT.value,
                               client_header,
                               self.key,
                               Code.OK.value)
            self.sock.send(resp)

        else:
            logger.warning("Invalid message {}".format(msg.hex()))


def run_main():
    main.run_push()
    main.run_pull()
    main.set_max_poll(emerg_max_poll)


if __name__ == "__main__":
    host = "110.85.26.60"
    port = 7895
    # elevator_id = "22785001"
    default_chan = "22785001"
    is_debug = True
    base_rtmp_url = "rtmp://{}:1935/live/".format(host)
    opts_dict = {
        "input_file_path": "0",
        # "input_file_path": os.path.join(pwd, "test.mp4"),
        # "input_file_path": "/home/crosstyan/Code/ncnn/py/test (115).mp4",
        "param_path": os.path.join(pwd, "..", "model", "yolo-fastestv2-opt.param"),
        "bin_path": os.path.join(pwd, "..", "model", "yolo-fastestv2-opt.bin"),
        "rtmp_url": base_rtmp_url + default_chan,
        "redis_url": "tcp://127.0.0.1:6379",
        "target_input_width": 640,
        "target_input_height": 480,
        "target_input_fps": -1,
        "scaled_coeffs": 0.2,
        "threshold_NMS": 0.125,
        "out_fps": 12,
        "threads_num": 4,
        "is_border": False,
        "is_debug": is_debug,
    }

    opts = yolo_app.init_options(opts_dict)
    main = yolo_app.MainWrapper(opts)
    elevator_id = bytes.fromhex("8888888888888888")
    main.init()
    u = UDPApp(host, port, main, elevator_id)
    main.set_on_detect_yolo(u.on_detect_yolo)
    main.set_on_detect_door(u.on_detect_door)
    main.set_on_poll_complete(u.on_poll_complete)
    g = gevent.spawn(u.serve_forever)
    run_g = gevent.spawn(run_main)
    s = gevent.spawn(u.send_status_forever)
    send_init = gevent.spawn(u.send_init_req)
    g.start()
    gevent.joinall([g, send_init, s, run_g])
