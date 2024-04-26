#include "RtmpWriter.h"
// #include "Conf.h"
#include "UtilRtmp.h"
#include <stdio.h>
#include "log.hpp"
#include "sps.hpp"

#if defined(SRS_LIBRTMP)
RtmpWriter::RtmpWriter() {
  _avg726.init(40000, G726_PACKING_RIGHT);
  _startVTime = 0;
  _startATime = 0;
}

RtmpWriter::~RtmpWriter() {
  srs_rtmp_destroy(_rtmp);
  _g726ToAac.uinit();
  _avg726.uninit();
  _startVTime = 0;
  _startATime = 0;
}

const char* RtmpWriter::errorMsg() { return _errMsg; }

int RtmpWriter::errorCode() { return _errCode; }

bool RtmpWriter::initUrl(const char* url) {
  printf("%s\n", url);
  _rtmp = srs_rtmp_create(url);
  if (srs_rtmp_handshake(_rtmp) != 0) {
    _errMsg = "simple handshake failed.";
    return false;
  }
  if (srs_rtmp_connect_app(_rtmp) != 0) {
    _errMsg = "connect vhost/app failed.";
    return false;
  }
  if (srs_rtmp_publish_stream(_rtmp) != 0) {
    _errMsg = "publish stream failed.";
    return false;
  }
  _errCode = 0;
  _fpName = url;
  std::string::size_type m = _fpName.rfind("/", _fpName.length());
  _fpWriter.initfp(&_fpName.at((int)m + 1));
  return true;
}

// 这里只能上产到srs服务，nginx不行
void RtmpWriter::publishVideoframe(char* frame, int len, int type,
                                   unsigned long long pts) {
  if (_startVTime == 0) {
    _startVTime = pts;
  }
  uint32_t dts = (pts - _startVTime) / 1000;
  _errCode = srs_h264_write_raw_frames(_rtmp, frame, len, dts, dts);
  if (srs_h264_is_dvbsp_error(_errCode)) {
    _errMsg = "ignore drop video error";
  } else if (srs_h264_is_duplicated_sps_error(_errCode)) {
    _errMsg = "ignore duplicated sps";
  } else if (srs_h264_is_duplicated_pps_error(_errCode)) {
    _errMsg = "ignore duplicated pps";
  } else {
    _errMsg = "send h264 raw data failed";
  }
}

// G726 转 pcm
void RtmpWriter::publishAudioframe(char* frame, int len,
                                   unsigned long long pts) {
  // 海思G726转PCM
  char ampBuf[1024] = {0};
  int pcmLen = _avg726.decodec(frame, len, ampBuf);
  if (pcmLen != 640) {
    return;
  }
  // 直接发送pcm
  srs_audio_write_raw_frame(_rtmp, 3, 3, 1, 0, ampBuf, pcmLen,
                            (pts - _startVTime) / 1000);
  return;
  // _fpWriter.write(frame, len);
  // // 发送AAC
  // if (_startATime == 0) {
  //     _startATime = pts;
  //     _g726ToAac.init();
  // }
  // int            aacLen = 0;
  // unsigned char* aac = _g726ToAac.toAacEncodec(frame, len, aacLen);
  // if (aacLen == 0) {
  //     return;
  // }
  // uint32_t dts = (pts - _startVTime) / 1000;
  // srs_audio_write_raw_frame(_rtmp, 10, 1, 1, 1, ( char* )aac, aacLen, dts);
}

#else

RtmpWriter::RtmpWriter() {
  _isWaitKeyframe = true;
  _frameType = 0;
  _startVTime = 0;
  _startATime = 0;
}
RtmpWriter::~RtmpWriter() {
  if (_rtmp) {
    RTMP_Close(_rtmp);
    RTMP_Free(_rtmp);
    _rtmp = NULL;
  }
  _isWaitKeyframe = true;
  _frameType = 0;
  _startATime = 0;
  _startVTime = 0;
  _g726ToAac.uinit();
}

const char* RtmpWriter::errorMsg() { return _errMsg; }

int RtmpWriter::errorCode() { return _errCode; }

bool RtmpWriter::initUrl(const char* url) {
  LogDebug("%s\n", url);
  do {
    _rtmp = RTMP_Alloc();
    RTMP_Init(_rtmp);
    // set connection timeout,default 30s
    if (!RTMP_SetupURL(_rtmp, (char*)url)) {
      _errMsg = "SetupURL Error";
      break;
    }
    //
    RTMP_EnableWrite(_rtmp);
    if (0 == RTMP_Connect(_rtmp, NULL)) {
      _errMsg = "Connect Server Error";
      break;
    }
    // 连接流
    if (0 == RTMP_ConnectStream(_rtmp, 0)) {
      _errMsg = "Connect Stream Error";
      RTMP_Close(_rtmp);
      break;
    }
    RTMP_LogSetLevel(RTMP_LOGCRIT);  // 不显示打印日志
    // _fpName = url;
    // std::string::size_type m = _fpName.rfind("/", _fpName.length());
    // std::string            devID = &_fpName.at(( int )m + 1);
    // _g726Writer.initfp((devID.append(".g726").c_str()));
    // _mp4Writer.fpname(devID.c_str());
    return true;
  } while (0);
  RTMP_Free(_rtmp);
  _rtmp = NULL;
  return false;
}

// 推流
void RtmpWriter::publishVideoframe(char* frame, size_t len, int type,
                                   long pts) {
  // 关键帧
  if (_isWaitKeyframe && type == 0x01) {
    unsigned char type = frame[4] & 0x1F;
    // 0x01 slice 0x05 idr 0x06 sei 0x07 sps 0x08 pps
    switch (type) {
      case avc::NALU_TYPE_IDR:
      case avc::NALU_TYPE_SPS:
      case avc::NALU_TYPE_PPS:
      case avc::NALU_TYPE_SLICE:
      case avc::NALU_TYPE_SEI:
        _frameType = 264;
        _publishFunc = Send264Videoframe;
        break;
      default:
        _frameType = 265;
        _publishFunc = Send265Videoframe;
        break;
    }
    _startVTime = pts;
    _isWaitKeyframe = false;
  }
  if (_frameType == 0) {
    return;
  }
  std::vector<nalu> nalus;
  char* ptr = naluparse(frame, len, nalus);
  // _mp4Writer.writeVideoframe(frame, len, type, pts);
  uint32_t dts = (pts - _startVTime) / 1000;
  for (const auto& it : nalus) {
    _publishFunc(_rtmp, it.data, it.size, dts);
  }
  _publishFunc(_rtmp, ptr, len, dts);
}

// 发送AAC RTMP包
// 可以参考 http://billhoo.blog.51cto.com/2337751/1557646/
void RtmpWriter::publishAudioframe(char* frame, size_t len, long pts) {
  //_mp4Writer.writeAudioframe(frame, len, pts);
  if (_startATime == 0) {
    _startATime = pts;
    _g726ToAac.init();
  }
  // _g726Writer.write(frame, len);
  int aacLen = 0;
  unsigned char* aac = _g726ToAac.toAacEncodec(frame, len, aacLen);
  if (aacLen == 0) {
    return;
  }
  uint32_t dts = (pts - _startATime) / 1000;
  static bool isSeedSpecialInfo = false;
  if (!isSeedSpecialInfo) {
    int specialLen = 0;
    unsigned char* pSpecialData = _g726ToAac.aacSpecialData(&specialLen);
    SendAccAudioframe(_rtmp, pSpecialData, specialLen, dts, false);
    isSeedSpecialInfo = true;
  }
  SendAccAudioframe(_rtmp, aac, aacLen, dts);
}

#endif