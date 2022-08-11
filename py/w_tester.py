
from subprocess import call
import gevent
from gevent import socket, Greenlet, sleep
from toolz import mapcat as flatmap
from w_helper import *
import struct
import argparse
import datetime
from loguru import logger


class UDPApp:
    def __init__(self, remote_host: str, remote_port: int, yolo_app, id: bytes):
        self.id = id
        self.host = remote_host
        self.port = remote_port
        self.status = Code.OK
        address = (remote_host, remote_port)
        # yolo_app is MainWrapper
        self.key:   None | bytes = None
        self.height = 0
        self.dist = 0
        # self.e_chan:  None | str = None
        self.app = yolo_app
        self.sock = socket.socket(type=socket.SOCK_DGRAM)
        self.sock.connect(address)

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
      
    def send_status(self):
        info = {
          "door_open": 0,
          "door_close": 0,
          "people_num": 0,
          "floor": 5,
          "call": 0,
          "x": 110, # mul 100 (fixed point)
          "y": 240,
          "z": 410,
          "ang_x": 980,
          "ang_y": 280,
          "ang_z": 420,
          "network": 0,
          "signal": 10,
          "error": 0,
          "battery": 38,
          "speed": 180, # mul 100 (fixed point)
          "pressure": 100560.47272811741
        }
        req = struct.pack(MsgStruct.STATUS_CLIENT.value,
                          MsgType.STATUS.value,
                          self.key,
                          info["door_open"],
                          info["door_close"],
                          info["people_num"],
                          0, 0, 0, 0, 0,
                          info["floor"],
                          info["call"],
                          info["x"],
                          info["y"],
                          info["z"],
                          info["ang_x"],
                          info["ang_y"],
                          info["ang_z"],
                          info["network"],
                          info["signal"],
                          info["error"],
                          info["battery"],
                          info["speed"],
                          info["pressure"])
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
          ## timestamp is unix timestamp
            _head, error, timestamp = struct.unpack(
                MsgStruct.STATUS_SERVER.value, msg)
            unix_timestamp =  int.from_bytes(timestamp, byteorder='big')
            dt = datetime.datetime.fromtimestamp(unix_timestamp)
            logger.info("{} {} from {}".format(error,dt.strftime('%Y-%m-%d %H:%M:%S'), msg.hex()))
        else:
            logger.warning("Invalid message {}".format(msg.hex()))

if __name__ == "__main__":
    host = "110.85.26.60"
    port = 7895
    elevator_id = "22785001"
    u = UDPApp(host, port, None, elevator_id.encode())
    g = gevent.spawn(u.serve_forever)
    s = gevent.spawn(u.send_status_forever)
    send_init = gevent.spawn(u.send_init_req)
    g.start()
    gevent.joinall([g, send_init, s])
