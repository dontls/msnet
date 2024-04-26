#ifndef RTMP_WRITER_H
#define RTMP_WRITER_H

#ifdef SRS_LIBRTMP
#include "srs/srs_librtmp.h"
#else
#include "librtmp/rtmp.h"
#include "librtmp/log.h"
#endif
#include "AVg726ToAac.h"
// #include "AVmp4.h"
// #include "fpWriter.h"
#include <functional>
#include <string>

class RtmpWriter {
 public:
 private:
  AVg726ToAac _g726ToAac;
  const char* _errMsg;
  int _errCode;
  long long _startVTime;
  long long _startATime;
  bool _isWaitKeyframe;
  int _frameType;
  // 发布流
#ifdef SRS_LIBRTMP
  srs_rtmp_t _rtmp;
  AVg726 _avg726;
#else
  RTMP* _rtmp;
  std::function<int(RTMP*, char*, int, uint32_t)> _publishFunc;
#endif
  // fpWriter    _g726Writer;
  // AVmp4       _mp4Writer;
  std::string _fpName;

 public:
  RtmpWriter();
  ~RtmpWriter();

  bool initUrl(const char* url);
  const char* errorMsg();
  int errorCode();

  void publishVideoframe(char* frame, size_t len, int type, long pts);
  void publishAudioframe(char* frame, size_t len, long pts);
};
#endif