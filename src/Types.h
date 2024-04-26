#ifndef __TYPES_H__
#define __TYPES_H__

namespace ho {
const int MsgHeaderLen = 8;
const char MsgVersion = 1;
const char MsgFlag = 'H';

#pragma pack(4)
typedef struct MsgHeader {
  unsigned char ucFlag;
  unsigned char ucVersion;
  unsigned short usCodeID;
  unsigned int unPayloadLen;
} MsgHeader_t;

typedef struct MediaHeader {
  unsigned short usFrameType;
  unsigned short usFrameChannel;
  unsigned long long ullFrameTimeStamp;
} MediaHeader_t;

#pragma pack()

enum {
  KFrameType_Invalid = 0x0000,  // 无效			//invalid
  KFrameType_Video_I = 0x0001,  // 视频I帧		//video i frame
  KFrameType_Video_P = 0x0002,  // 视频P帧		//video P frame
  KFrameType_Audio = 0x0003,    // 音频帧			//audio frame
};
}  // namespace ho

#endif
