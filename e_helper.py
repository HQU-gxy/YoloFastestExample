from enum import Enum

class MsgType(Enum):
  INIT = bytes.fromhex("70")
  RTMP_EMERG = bytes.fromhex("77")
  RTMP_STREAM = bytes.fromhex("75")
  HEARTBEAT = bytes.fromhex("64")

# for bytes reading
class ElemLen(Enum):
  ID   = 2
  HASH = 4
  RTMP = 2
