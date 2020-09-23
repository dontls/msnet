
#include "Publisher.h"
#include "Log.h"
#include "Sps.h"
#include <stdio.h>

Publisher::Publisher()
{
    _isWaitKeyframe = true;
    _frameFormatType = 0;
    _startAVTime = 0;
}
Publisher::~Publisher()
{
    _isWaitKeyframe = true;
    _startAVTime = 0;
    if (_g726ToAac) {
        _g726ToAac->uinit();
    }
}

bool Publisher::initSession(std::string session)
{
    _session = session;
    this->initAac();
    return this->initWriter();
}

// 设置视频流
void Publisher::recvVideoRaw(char* frame, int len, int type, unsigned long long pts)
{
    // 关键帧
    if (_isWaitKeyframe && type == 0x01) {
        frameFormat(frame);
        _startAVTime = pts;
    }
    if (_isWaitKeyframe) {
        return;
    }
    std::vector<std::string> naluVec = ParseNalUnit(frame, len);
    int                      naluSize = naluVec.size();
    if (naluSize < 0) {
        return;
    }
    // _mp4Writer.writeVideoframe(frame, len, type, pts);
    uint32_t apts = (pts - _startAVTime) / 1000;
    for (int i = 0; i < naluSize; i++) {
        if (_frameFormatType == VFRAME_TYPE_ID_HEVC) {
            publishHevcNaluRaw(naluVec[i], apts);
        } else {
            publishAvcNaluRaw(naluVec[i], apts);
        }
    }
}

// 发送AAC RTMP包
// 可以参考 http://billhoo.blog.51cto.com/2337751/1557646/
void Publisher::recvAudioRaw(char* frame, int len, unsigned long long pts)
{
    uint32_t       abstamp = (pts - _startAVTime) / 1000;
    int            aacLen = 0;
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

void Publisher::publishAvcNaluRaw(std::string& naluRaw, uint32_t abstamp)
{
    const char* frameBuf = naluRaw.c_str();
    std::string ostr = "";
    int         frameLen = naluRaw.length();
    int         nut = frameBuf[0] & 0x1F;
    bool        isMeta = false;
    switch (nut) {
    case avc::NALU_TYPE_SPS:
        _avcExt.sps = naluRaw;
        break;
    case avc::NALU_TYPE_PPS:
        _avcExt.pps = naluRaw;
        if (_avcExt.sps.empty() || _avcExt.pps.empty()) {
            break;
        }
        ostr = _flvTag.flvMetaTag(_avcExt.sps.c_str(), _avcExt.sps.length(), _avcExt.pps.c_str(), _avcExt.pps.length());
        isMeta = true;
        break;
    case avc::NALU_TYPE_SLICE:
        ostr = _flvTag.flvVideoTag(frameBuf, frameLen, false);
        break;
    case avc::NALU_TYPE_IDR:
        ostr = _flvTag.flvVideoTag(frameBuf, frameLen, true);
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

void Publisher::publishHevcNaluRaw(std::string& naluRaw, uint32_t abstamp)
{
    const char* frameBuf = naluRaw.c_str();
    int         frameLen = naluRaw.length();
    int         nut = (frameBuf[0] & 0x7E) >> 1;
    std::string ostr = "";
    bool        isMeta = false;
    switch (nut) {
    case hevc::NAL_UNIT_VPS:
        _avcExt.vps = naluRaw;
        break;
    case hevc::NAL_UNIT_SPS:
        _avcExt.sps = naluRaw;
        break;
    case hevc::NAL_UNIT_PPS:
        _avcExt.pps = naluRaw;
        if (_avcExt.sps.empty() || _avcExt.pps.empty() || _avcExt.vps.empty()) {
            break;
        }
        ostr = _flvTag.flvHevcMetaTag(_avcExt.vps.c_str(), _avcExt.vps.length(), _avcExt.sps.c_str(),
                                      _avcExt.sps.length(), _avcExt.pps.c_str(), _avcExt.pps.length());
        isMeta = true;
        break;
    case hevc::NAL_UNIT_CODED_SLICE_TRAIL_R:
        ostr = _flvTag.flvVideoTag(frameBuf, frameLen, false, true);
        break;
    case hevc::NAL_UNIT_CODED_SLICE_IDR:
        ostr = _flvTag.flvVideoTag(frameBuf, frameLen, true, true);
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
void Publisher::frameFormat(char* frame)
{
    unsigned char type = frame[4] & 0x1F;
    // 0x01 slice 0x05 idr 0x06 sei 0x07 sps 0x08 pps
    switch (type) {
    case avc::NALU_TYPE_IDR:
    case avc::NALU_TYPE_SPS:
    case avc::NALU_TYPE_PPS:
    case avc::NALU_TYPE_SLICE:
    case avc::NALU_TYPE_SEI:
        _frameFormatType = VFRAME_TYPE_ID_H264;
        break;
    default:
        _frameFormatType = VFRAME_TYPE_ID_HEVC;
        break;
    }
    _isWaitKeyframe = false;
}

void Publisher::initAac()
{
    _g726ToAac = new AVg726ToAac();
    if (_g726ToAac) {
        _g726ToAac->init();
    }
    int            specLen = 0;
    unsigned char* specData = _g726ToAac->aacSpecialData(&specLen);
    _aacFlvTagSpec = _flvTag.flvAudioTag(specData, specLen, true);
}

bool Publisher::initWriter()
{
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