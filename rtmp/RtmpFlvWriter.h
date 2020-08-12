#ifndef RTMP_FLVWRITER_H
#define RTMP_FLVWRITER_H

#include "Conf.h"
#include "Log.h"
#include "librtmp/log.h"
#include "rtmp.h"
#include "utils/LlocBytes.h"
#include <string.h>
#include <string>

#define RTMP_HEAD_SIZE (sizeof(RTMPPacket) + RTMP_MAX_HEADER_SIZE)

class RtmpFlvWriter {
private:
    RTMP*       _rtmp;
    const char* _errMsg;
    LlocBytes   _packet;

    bool initUrl(const char* url)
    {
        LOG("%s\n", url);
        do {

            _rtmp = RTMP_Alloc();
            RTMP_Init(_rtmp);
            // set connection timeout,default 30s
            if (!RTMP_SetupURL(_rtmp, ( char* )url)) {
                _errMsg = "SetupURL Error";
                break;
            }
            //
            RTMP_EnableWrite(_rtmp);
            if (0 == RTMP_Connect(_rtmp, NULL)) {
                _errMsg = "Connect Server Error";
                break;
            }
            //连接流
            if (0 == RTMP_ConnectStream(_rtmp, 0)) {
                _errMsg = "Connect Stream Error";
                RTMP_Close(_rtmp);
                break;
            }
            //不显示打印日志
            RTMP_LogSetLevel(RTMP_LOGCRIT);
            // _fpName = url;
            // std::string::size_type m = _fpName.rfind("/", _fpName.length());
            // std::string            devID = &_fpName.at(( int )m + 1);
            // _g726Writer.initfp((devID.append(".g726").c_str()));
            // _mp4Writer.fpname(devID.c_str());
            return true;
        } while (0);
        RTMP_Close(_rtmp);
        RTMP_Free(_rtmp);
        _rtmp = NULL;
        LOG("%s\n", _errMsg);
        return false;
    }

    int sendPacket(unsigned int nPacketType, const char* body, int bodyLen, uint32_t abstamp)
    {
        if (!_rtmp) {
            return -1;
        }
        char*       tBuffer = _packet.Newlloc(RTMP_HEAD_SIZE + bodyLen);
        RTMPPacket* packet = ( RTMPPacket* )tBuffer;
        // RTMPPacket_Reset(packet);//重置packet状态
        packet->m_body = ( char* )packet + RTMP_HEAD_SIZE;
        packet->m_nBodySize = bodyLen;
        memcpy(packet->m_body, body, bodyLen);
        packet->m_hasAbsTimestamp = 0;
        packet->m_packetType = nPacketType; /*此处为类型有两种一种是音频,一种是视频*/
        packet->m_nInfoField2 = _rtmp->m_stream_id;
        packet->m_nChannel = 0x04;
        packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
        if (RTMP_PACKET_TYPE_AUDIO == nPacketType) {
            packet->m_nChannel = 0x05;
            packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
        }
        packet->m_nTimeStamp = abstamp;
        /*调用发送接口*/
        int nRet = 0;
        if (RTMP_IsConnected(_rtmp)) {
            nRet = RTMP_SendPacket(_rtmp, packet, TRUE);
        }
        return nRet;
    }

public:
    RtmpFlvWriter(/* args */) {}
    ~RtmpFlvWriter()
    {
        if (_rtmp) {
            RTMP_Close(_rtmp);
            RTMP_Free(_rtmp);
            _rtmp = NULL;
        }
    }
    bool init(const char* session)
    {
        std::string rtmpUrl = GetRtmpBaseUrl();
        if (!initUrl(rtmpUrl.append(session).c_str())) {
            return false;
        }
        return true;
    }
    void setAvcRtmpPacket(std::string flvTag, uint32_t abstamp)
    {
        sendPacket(RTMP_PACKET_TYPE_VIDEO, flvTag.c_str(), flvTag.length(), abstamp);
    }

    void setAccRtmpPacket(std::string flvTag, uint32_t abstamp)
    {
        sendPacket(RTMP_PACKET_TYPE_AUDIO, flvTag.c_str(), flvTag.length(), abstamp);
    }
};

typedef std::shared_ptr<RtmpFlvWriter> RtmpFlvWriter_Ptr;

#endif