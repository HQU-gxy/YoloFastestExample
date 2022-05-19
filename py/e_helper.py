from enum import Enum

## Avoid using value > 0x80 (128)
class MsgType(Enum):
  INIT = bytes.fromhex("70")[0]
  RTMP_EMERG = bytes.fromhex("77")[0]
  RTMP_STREAM = bytes.fromhex("75")[0]
  RTMP_STOP = bytes.fromhex("76")[0]
  HEARTBEAT = bytes.fromhex("64")[0]
  PRESSURE =  bytes.fromhex("90")[0]
  ACC =  bytes.fromhex("88")[0]

# See https://docs.python.org/3/library/struct.html
class MsgStruct(Enum):
  INIT_CLIENT = ">BH" # type id
  INIT_SERVER = ">BI" # type hash

  RTMP_EMERG_CLIENT = ">BI" # type hash
  RTMP_EMERG_SERVER = ">BIH" # type hash chann

  RTMP_STREAM_SERVER = ">BIH" # type hash chann
  RTMP_STREAM_CLIENT = ">BIHB" # type hash chann reason

  RTMP_STOP_SERVER = ">BI" # type hash
  RTMP_STOP_CLIENT = ">BIB" # type hash reason

  HEARTBEAT_CLIENT = ">BI" # type hash

  PRESSURE_CLIENT = ">BIf" # type hash val

  ACC_CLIENT = ">BIfff" # type hash x y z

class Code(Enum):
  OK = bytes.fromhex("ff")[0]
  BUSY = bytes.fromhex("10")[0]
  BUSY_EMERG = bytes.fromhex("11")[0]
  BUSY_STREAM = bytes.fromhex("12")[0]
  ERR = bytes.fromhex("00")[0]

# Use condp from https://github.com/jblomo/CljPy/blob/master/cljpy/core.py
