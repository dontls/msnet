#ifndef PUBLISHER_H
#define PUBLISHER_H

#include <memory>

#include "AVg726ToAac.h"
#include "flv.hpp"
#include "flv/FlvWriter.h"
#include "rtmp.hpp"

#define USE_FLV_SRV

#include <string>
class Publisher {
 private:
  AVg726ToAac* g726ToAac_;
  long long firts_;
  bool bSpecAac_;
  std::string flvAacSpecal_;
  std::string session_;
  libflv::Packet flvPkt_;
  librtmp::Client rtmp_;
  FlvWriter_Ptr flvWriter_;

 public:
  Publisher();
  ~Publisher();
  bool initSession(std::string session);
  void onRawVideo(char* frame, size_t len, int type, long long pts);
  void onRawAudio(char* frame, size_t len, long long pts);

 private:
  bool initWriter();
  void initAac();
};

typedef std::shared_ptr<Publisher> Publisher_Ptr;

#endif