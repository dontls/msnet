
#include "Publisher.h"
#include "rtmp.hpp"
#include "sps.hpp"
#include <stdio.h>
#include "Conf.h"

Publisher::Publisher() : flvPkt_(RTMP_HEAD_SIZE) {
  bSpecAac_ = true;
  firts_ = 0;
}
Publisher::~Publisher() {
  if (g726ToAac_) {
    g726ToAac_->uinit();
  }
}

bool Publisher::initSession(std::string session) {
  session_ = session;
  this->initAac();
  return this->initWriter();
}

// 设置视频流
void Publisher::onRawVideo(char* frame, size_t len, int type, long long pts) {
  if (firts_ == 0 && type == 1) {
    firts_ = pts;
  }
  if (firts_ == 0) {
    return;
  }
  nalu::Vector nalus;
  char* data = nalu::Split(frame, len, nalus);
  if (data == nullptr) {
    return;
  }
  uint32_t dts = (pts - firts_) / 1000;
  if (type == 1) {
    rtmp_.WritePacket(0x09, flvPkt_.Marshal(nalus), dts);
#if defined(USE_FLV_SRV)
    libyte::Slice s = flvPkt_.Pack(0x09, dts);
    flvWriter_->WriteFrame(s.data, s.length, true);
#endif
  }
  rtmp_.WritePacket(0x09, flvPkt_.Marshal(data, len), dts);
#if defined(USE_FLV_SRV)
  libyte::Slice s = flvPkt_.Pack(0x09, dts);
  flvWriter_->WriteFrame(s.data, s.length, false);
#endif
}

// 发送AAC RTMP包
// 可以参考 http://billhoo.blog.51cto.com/2337751/1557646/
void Publisher::onRawAudio(char* frame, size_t len, long long pts) {
  if (firts_ == 0) {
    return;
  }
  uint32_t dts = (pts - firts_) / 1000;
  if (bSpecAac_) {
    libyte::Buffer* b = flvPkt_.Marshal(flvAacSpecal_);
    rtmp_.WritePacket(0x08, b, dts);
    bSpecAac_ = false;
  }
  int aacLen = 0;
  size_t dlen = 0;
  char* aac = g726ToAac_->toAacEncodec(frame, len, aacLen);
  if (aacLen == 0) {
    return;
  }
  rtmp_.WritePacket(0x08, flvPkt_.Marshal(aac, aacLen), dts);
#if defined(USE_FLV_SRV)
  libyte::Slice s = flvPkt_.Pack(0x08, dts);
  flvWriter_->WriteFrame(s.data, s.length, false);
#endif
}

void Publisher::initAac() {
  g726ToAac_ = new AVg726ToAac();
  if (g726ToAac_) {
    g726ToAac_->init();
  }
  int specLen = 0;
  char* specData = g726ToAac_->aacSpecialData(&specLen);
  flvAacSpecal_ = std::string(specData, specLen);
}

bool Publisher::initWriter() {
  // rtmp
  std::string rtmpUrl = GetRtmpBaseUrl();
  rtmp_.Dial(rtmpUrl.append(session_).c_str());
  // flv 0x09 tagType vedio 0x08 tagType audio
#if defined(USE_FLV_SRV)
  if (!flvWriter_) {
    flvWriter_ = FlvWriterManager::ins()->flvWriter(session_);
    flvPkt_.Marshal(flvAacSpecal_);
    libyte::Slice b = flvPkt_.Pack(0x08, 0);
    std::string s(b.data, b.length);
    flvWriter_->WriteAACSpec(s);
  }
#endif
  return true;
}