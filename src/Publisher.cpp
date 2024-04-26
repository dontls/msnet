
#include "Publisher.h"
#include "log.hpp"
#include "sps.hpp"
#include <stdio.h>

Publisher::Publisher() {
  _isWaitKeyframe = true;
  _frameFormatType = 0;
  _startAVTime = 0;
}
Publisher::~Publisher() {
  _isWaitKeyframe = true;
  _startAVTime = 0;
  if (_g726ToAac) {
    _g726ToAac->uinit();
  }
}

bool Publisher::initSession(std::string session) {
  _session = session;
  this->initAac();
  return this->initWriter();
}

// 设置视频流
void Publisher::recvVideoRaw(char* frame, size_t len, int type, long long pts) {
  // 关键帧
  if (_isWaitKeyframe && type == 0x01) {
    frameFormat(frame);
    _startAVTime = pts;
  }
  if (_isWaitKeyframe) {
    return;
  }
  std::vector<nalu> nalus;
  char* ptr = naluparse(frame, len, nalus);
  // _mp4Writer.writeVideoframe(frame, len, type, pts);
  long apts = (pts - _startAVTime) / 1000;
  for (const auto& it : nalus) {
    _publishFunc(it.data+4, it.size-4, apts);
  }
  _publishFunc(ptr+4, len-4, apts);
}

// 发送AAC RTMP包
// 可以参考 http://billhoo.blog.51cto.com/2337751/1557646/
void Publisher::recvAudioRaw(char* frame, size_t len, long long pts) {
  uint32_t abstamp = (pts - _startAVTime) / 1000;
  int aacLen = 0;
  unsigned char* aac = _g726ToAac->toAacEncodec(frame, len, aacLen);
  if (aacLen == 0) {
    return;
  }
  std::string ostr = _flvTag.flvAudioTag(aac, aacLen);

  _rtmpFlvWriter->setAvcRtmpPacket(ostr, abstamp);
#if defined(USE_FLV_SRV)
  // flv 0x09 tagType vedio 0x08 tagType audio
  _flvWriter->setFlvPacket(0x08, ostr, abstamp);
#endif
}

void Publisher::publishAvcNaluRaw(char* data, size_t len, long abstamp) {
  std::string ostr = "";
  int nut = data[0] & 0x1F;
  bool isMeta = false;
  switch (nut) {
    case avc::NALU_TYPE_SPS:
      _avcExt.sps = std::string(data, len);
      break;
    case avc::NALU_TYPE_PPS:
      _avcExt.pps = std::string(data, len);
      if (_avcExt.sps.empty() || _avcExt.pps.empty()) {
        break;
      }
      ostr = _flvTag.flvMetaTag(_avcExt.sps.c_str(), _avcExt.sps.length(),
                                _avcExt.pps.c_str(), _avcExt.pps.length());
      isMeta = true;
      break;
    case avc::NALU_TYPE_SLICE:
      ostr = _flvTag.flvVideoTag(data, len, false);
      break;
    case avc::NALU_TYPE_IDR:
      ostr = _flvTag.flvVideoTag(data, len, true);
      break;
  }
  if (ostr == "") {
    return;
  }
  // rtmp 已经处理过i帧不需要等
  _rtmpFlvWriter->setAvcRtmpPacket(ostr, abstamp);

#if defined(USE_FLV_SRV)
  // flv 0x09 tagType vedio 0x08 tagType audio
  _flvWriter->setFlvPacket(0x09, ostr, abstamp, isMeta);
#endif
}

void Publisher::publishHevcNaluRaw(char* data, size_t len, long abstamp) {
  int nut = (data[0] & 0x7E) >> 1;
  std::string ostr = "";
  bool isMeta = false;
  switch (nut) {
    case hevc::NALU_TYPE_VPS:
      _avcExt.vps = std::string(data, len);
      break;
    case hevc::NALU_TYPE_SPS:
      _avcExt.sps = std::string(data, len);
      break;
    case hevc::NALU_TYPE_PPS:
      _avcExt.pps = std::string(data, len);
      if (_avcExt.sps.empty() || _avcExt.pps.empty() || _avcExt.vps.empty()) {
        break;
      }
      ostr = _flvTag.flvHevcMetaTag(_avcExt.vps.c_str(), _avcExt.vps.length(),
                                    _avcExt.sps.c_str(), _avcExt.sps.length(),
                                    _avcExt.pps.c_str(), _avcExt.pps.length());
      isMeta = true;
      break;
    case hevc::NALU_TYPE_CODED_SLICE_TRAIL_R:
      ostr = _flvTag.flvVideoTag(data, len, false, true);
      break;
    case hevc::NALU_TYPE_CODED_SLICE_IDR:
      ostr = _flvTag.flvVideoTag(data, len, true, true);
      break;
    default:
      break;
  }
  if (ostr == "") {
    return;
  }
  // rtmp 已经处理过i帧不需要等
  _rtmpFlvWriter->setAvcRtmpPacket(ostr, abstamp);

#if defined(USE_FLV_SRV)
  // flv 0x09 tagType vedio 0x08 tagType audio
  _flvWriter->setFlvPacket(0x09, ostr, abstamp, isMeta);
#endif
}
//
void Publisher::frameFormat(char* frame) {
  unsigned char type = frame[4] & 0x1F;
  // 0x01 slice 0x05 idr 0x06 sei 0x07 sps 0x08 pps
  switch (type) {
    case avc::NALU_TYPE_IDR:
    case avc::NALU_TYPE_SPS:
    case avc::NALU_TYPE_PPS:
    case avc::NALU_TYPE_SLICE:
    case avc::NALU_TYPE_SEI:
      _frameFormatType = 264;
      // _publishFunc = publishAvcNaluRaw;
      break;
    default:
      _frameFormatType = 265;
      // _publishFunc = publishHevcNaluRaw;
      break;
  }
  _isWaitKeyframe = false;
}

void Publisher::initAac() {
  _g726ToAac = new AVg726ToAac();
  if (_g726ToAac) {
    _g726ToAac->init();
  }
  int specLen = 0;
  unsigned char* specData = _g726ToAac->aacSpecialData(&specLen);
  _aacFlvTagSpec = _flvTag.flvAudioTag(specData, specLen, true);
}

bool Publisher::initWriter() {
  // rtmp
  _rtmpFlvWriter = std::make_shared<RtmpFlvWriter>();
  _rtmpFlvWriter->init(_session.c_str());
#if defined(USE_FLV_SRV)
  // flv 0x09 tagType vedio 0x08 tagType audio
  if (!_flvWriter) {
    _flvWriter = FlvWriterManager::ins()->flvWriter(_session);
    _flvWriter->setSpecificConfig(_aacFlvTagSpec);
  }
#endif
  return true;
}