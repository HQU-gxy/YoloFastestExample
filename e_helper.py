from enum import Enum

## Avoid using value > 0x80 (128)
class MsgType(Enum):
  INIT = bytes.fromhex("70")[0]
  RTMP_EMERG = bytes.fromhex("77")[0]
  RTMP_STREAM = bytes.fromhex("75")[0]
  HEARTBEAT = bytes.fromhex("64")[0]

# See https://docs.python.org/3/library/struct.html
class MsgStruct(Enum):
  INIT_CLIENT = ">BH" # type id
  INIT_SERVER = ">BI" # type hash

  RTMP_EMERG_CLIENT = ">BI" # type hash
  RTMP_EMERG_SERVER = ">BIH" # type hash chann

  RTMP_STREAM_SERVER = ">BIH" # type hash chann
  RTMP_STREAM_CLIENT = ">BIHB" # type hash chann reason

  HEARTBEAT_CLIENT = ">BI" # type hash

class Code(Enum):
  OK = bytes.fromhex("ff")[0]
  BUSY = bytes.fromhex("01")[0]
  ERR = bytes.fromhex("00")[0]
