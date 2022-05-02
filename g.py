from operator import is_
import os
import time
import sys
import gevent
from gevent import socket, Greenlet
from toolz import mapcat as flatmap
from e_helper import *
from typing import List, TypeVar
import hy
import asyncio # I know asyncio but there not too much udp library
# import logging
from loguru import logger
import struct
# https://docs.python.org/3/howto/logging.html

pwd = os.path.dirname(os.path.realpath(__file__))
print(pwd)
so_root = os.path.join(pwd, 'cmake-build-debug')
print(so_root)
sys.path.insert(0, so_root)

import yolo_app

base_rtmp_url = "rtmp://localhost:1935/live/"
default_chan = "unknown"

is_debug = True

emerg_max_poll = 60
stream_max_poll = 1500

opts_dict = {
  "input_file_path": "0",
  # "input_file_path": os.path.join(so_root, "test.mp4"),
  "output_file_path": "",
  "param_path": os.path.join(pwd, "model", "yolo-fastestv2-opt.param"),
  "bin_path": os.path.join(pwd, "model", "yolo-fastestv2-opt.bin"),
  "rtmp_url": base_rtmp_url + default_chan,
  "redis_url": "tcp://127.0.0.1:6379",
  "scaled_coeffs": 0.2,
  "threshold_NMS": 0.125,
  "out_fps": 6,
  "crop_coeffs": 0.1,
  "threads_num": 4,
  "is_debug": is_debug,
}


# https://loguru.readthedocs.io/en/stable/resources/recipes.html#changing-the-level-of-an-existing-handler
# https://github.com/Delgan/loguru/issues/138
loguru_level =  "DEBUG" if is_debug else "INFO"
logger.remove()
logger.add(sys.stderr, level=loguru_level)

base_pipeline = "appsrc ! " + \
                "videoconvert ! " + \
                "x264enc  pass=5 quantizer=25 speed-preset=6 ! " + \
                "video/x-h264, profile=baseline ! " + \
                "flvmux ! " + \
                "rtmpsink location="


# https://www.gevent.org/examples/udp_client.html
# https://www.gevent.org/examples/udp_server.html

# stream
# make sure server and client are share the same port

T = TypeVar("T")

def drop (coll: list[T], n: int=1)->list[T]:
  return coll[n:]

def take (coll: list[T], n: int=1)->list[T]:
  return coll[:n]

def bytes_to_uint_str(b: bytes)->str:
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
    self.hash:   None  | int = None
    self.e_chan:  None | str = None
    self.app = yolo_app
    self.sock = socket.socket(type=socket.SOCK_DGRAM)
    self.sock.connect(address)
  
  # should call this instead of self.app.reset_poll
  # when reset poll
  def reset_poll(self):
    self.app.reset_poll()
    self.app.set_max_poll(emerg_max_poll)
    self.status = Code.OK

  def on_detect_yolo(self, xs):
    if xs:
      for x in xs:
        # pts = [x.x1, x.y1, x.x2, x.y2]
        # byte_list = bytes.fromhex("70") + \
        #     bytes(flatmap(lambda x: x.to_bytes(
        #         2, 'big', signed=True), pts))
        logger.trace("({},{}) ({},{}) category: {} score: {}"
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
    self.status = Code.OK

  # https://docs.python.org/3/library/struct.html
  # https://www.educative.io/edpresso/what-is-the-python-struct-module
  def handle_req(self, msg: bytes):
    match list(msg):
      case [MsgType.INIT.value, *rest]:
        try: 
          hash: int
          _h, hash = struct.unpack(MsgStruct.INIT_SERVER.value, msg)
          self.hash = hash
          logger.info("set hash as {}".format(self.hash.to_bytes(4, 'big').hex()))
        except:
          logger.error("unpack hash error")
          raise Exception("Can't get hash")
      case [MsgType.RTMP_EMERG.value, *rest]:
        hash: int
        chan: int
        _h, hash, chan = struct.unpack(MsgStruct.RTMP_EMERG_SERVER.value, msg)
        if (self.hash and self.hash == hash):
          self.e_chan = chan.to_bytes(2, 'big').hex()
          logger.info("set channel as {}".format(self.e_chan))
      case [MsgType.RTMP_STREAM.value, *rest]:
        head: int
        hash: int
        chan: int
        head, hash, chan = struct.unpack(MsgStruct.RTMP_STREAM_SERVER.value, msg)
        if (self.hash and self.hash == hash):
          chn_s = chan.to_bytes(2, 'big').hex()
          logger.info("Receive RTMP Channel {}".format(chn_s))
          if (self.app.get_pull_task_state() == False):
            self.reset_poll()
            self.app.clear_queue()

            self.app.set_max_poll(stream_max_poll)
            self.app.start_poll(gen_pipeline(chn_s))

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
                                Code.BUSY.value)
            self.sock.send(resp)

      case [MsgType.RTMP_STOP.value, *rest]:
        head: int
        hash: int
        head, hash = struct.unpack(MsgStruct.RTMP_STOP_SERVER.value,msg)
        if (self.hash and self.hash == hash):
          self.reset_poll()
          resp = struct.pack(MsgStruct.RTMP_STOP_CLIENT.value,
                              head,
                              hash,
                              Code.OK.value)
          self.sock.send(resp)

      case _:
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


host = "127.0.0.1"
port = 12345


def run_main():
  main.run_push()
  main.run_pull()
  main.set_max_poll(emerg_max_poll)
  # gevent.sleep(10)
  # main.reset_poll()
  # main.start_poll(gen_pipeline(u.e_chan))
  # gevent.sleep(10)
  # u.send_request_e_chan()
  # gevent.sleep(1)
  # main.reset_poll()
  # main.start_poll(gen_pipeline(u.e_chan))


if __name__ == "__main__":
  opts = yolo_app.init_options(opts_dict)
  main = yolo_app.MainWrapper(opts)
  main.init()
  u = UDPApp(host, port, main, 123)
  main.set_on_detect_yolo(u.on_detect_yolo)
  main.set_on_detect_door(u.on_detect_door)
  main.set_on_poll_complete(u.on_poll_complete)
  # https://sdiehl.github.io/gevent-tutorial/
  g = gevent.spawn(u.serve_forever)
  send_init = gevent.spawn(u.send_init_req())
  run_g = gevent.spawn(run_main)
  g.start()
  gevent.joinall([g, send_init, run_g])

