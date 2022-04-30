from enum import Enum

## Avoid using value > 0x80 (128)
class MsgType(Enum):
  INIT = bytes.fromhex("70")[0]
  RTMP_EMERG = bytes.fromhex("77")[0]
  RTMP_STREAM = bytes.fromhex("75")[0]
  HEARTBEAT = bytes.fromhex("64")[0]

# for bytes reading
class ElemLen(Enum):
  ID   = 2
  HASH = 4
  RTMP_CHN = 2

class Code(Enum):
  OK = bytes.fromhex("ff")[0]
  BUSY = bytes.fromhex("01")[0]
  ERR = bytes.fromhex("00")[0]
