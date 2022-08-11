from enum import Enum

## Avoid using value > 0x80 (128)
class MsgType(Enum):
  INIT = bytes.fromhex("57")[0]
  STATUS = bytes.fromhex("68")[0]
  RTMP_EMERG = bytes.fromhex("80")[0]
  RTMP_STREAM = bytes.fromhex("90")[0]
  RTMP_STOP = bytes.fromhex("91")[0]

class MsgTypeServer(Enum):
  INIT = bytes.fromhex("77")[0]
  STATUS = bytes.fromhex("78")[0]
  RTMP_EMERG = bytes.fromhex("80")[0]
  RTMP_STREAM = bytes.fromhex("90")[0]
  RTMP_STOP = bytes.fromhex("91")[0]

# See https://docs.python.org/3/library/struct.html
class MsgStruct(Enum):
  INIT_CLIENT = ">B8sB" # type id(8bytes) end_call
  INIT_SERVER = ">BB16sBH" # type 0x00 key(16bytes) height dist

  RTMP_EMERG_CLIENT = ">B16s" # type key(16bytes)
  RTMP_EMERG_SERVER = ">BB16s" # type error key(16bytes)

  RTMP_STREAM_SERVER = ">B16s" # type channel(16bytes) (without key)
  RTMP_STREAM_CLIENT = ">B16sB" # type key(16bytes) error

  RTMP_STOP_SERVER = ">B" # type
  RTMP_STOP_CLIENT = ">B16sB" # type key(16bytes) error

  # h key door_open door_close number n/a*5 floor call xyz xyz(angle) network singal error battery speed pressure
  STATUS_CLIENT = ">B 16s BBBB BBBB BB hhh hhh BBBBB i" 
  STATUS_SERVER = ">BB4s" # h error timestamp(unix timestamp)
  

class Code(Enum):
  OK = bytes.fromhex("00")[0]
  BUSY = bytes.fromhex("01")[0]
  END = bytes.fromhex("2e")[0]

# Use condp from https://github.com/jblomo/CljPy/blob/master/cljpy/core.py
