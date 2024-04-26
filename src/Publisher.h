#ifndef PUBLISHER_H
#define PUBLISHER_H

#include <memory>

#include "AVg726ToAac.h"
#include "flv/FlvTag.h"
#include "flv/FlvWriter.h"
#include "rtmp/RtmpFlvWriter.h"
// #include "AVmp4.h"

#include <string>
class Publisher {
 private:
  struct AvcExtData_t {
    std::string pps;
    std::string sps;
    std::string vps;
  };

 private:
  AVg726ToAac* _g726ToAac;
  unsigned long long _startAVTime;
  bool _isWaitKeyframe;
  int _frameFormatType;
  std::string _session;
  RtmpFlvWriter_Ptr _rtmpFlvWriter;
  FlvWriter_Ptr _flvWriter;
  FlvTag _flvTag;
  AvcExtData_t _avcExt;
  std::string _aacFlvTagSpec;
  std::function<void(char*, size_t, long)> _publishFunc;

 public:
  Publisher();
  ~Publisher();
  bool initSession(std::string session);
  void recvVideoRaw(char* frame, size_t len, int type, long long pts);
  void recvAudioRaw(char* frame, size_t len, long long pts);

 private:
  void publishAvcNaluRaw(char* data, size_t len, long abstamp);
  void publishHevcNaluRaw(char* data, size_t len, long abstamp);
  bool initWriter();
  void initAac();
  void frameFormat(char* frame);
};

typedef std::shared_ptr<Publisher> Publisher_Ptr;

#endif