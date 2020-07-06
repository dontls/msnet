#ifndef __UTIL_RTMP_H__
#define __UTIL_RTMP_H__

#include "Sps.h"
#include "librtmp/rtmp.h"
#include <string.h>
#include <vector>

typedef struct AvcExtData_t {
    std::string pps;
    std::string sps;
    std::string vps;
} AvcExtData;

#define RTMP_HEAD_SIZE (sizeof(RTMPPacket) + RTMP_MAX_HEADER_SIZE)
// 发送视频的sps和pps信息
int SendVideoSpsPps(RTMP* r, unsigned char* pps, int pps_len, unsigned char* sps, int sps_len, uint32_t dts)
{
    char        tBuffer[RTMP_HEAD_SIZE + 1024] = { 0 };
    RTMPPacket* packet = ( RTMPPacket* )tBuffer;
    // RTMPPacket_Reset(packet);//重置packet状态
    packet->m_body = ( char* )packet + RTMP_HEAD_SIZE;
    unsigned char* body = ( unsigned char* )packet->m_body;
    int            i = 0;
    body[i++] = 0x17;
    i += 4;  // 4个字节0x00

    /*AVCDecoderConfigurationRecord*/
    body[i++] = 0x01;
    body[i++] = sps[1];
    body[i++] = sps[2];
    body[i++] = sps[3];
    body[i++] = 0xff;

    /*sps*/
    body[i++] = 0xe1;
    body[i++] = (sps_len >> 8) & 0xff;
    body[i++] = sps_len & 0xff;
    memcpy(&body[i], sps, sps_len);
    i += sps_len;

    /*pps*/
    body[i++] = 0x01;
    body[i++] = (pps_len >> 8) & 0xff;
    body[i++] = ( pps_len )&0xff;
    memcpy(&body[i], pps, pps_len);
    i += pps_len;

    packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    packet->m_nBodySize = i;
    packet->m_nChannel = 0x04;
    packet->m_nTimeStamp = dts;
    packet->m_hasAbsTimestamp = 0;
    packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
    packet->m_nInfoField2 = r->m_stream_id;
    /*调用发送接口*/
    int nRet = 0;
    if (RTMP_IsConnected(r)) {
        nRet = RTMP_SendPacket(r, packet, TRUE);
    }
    return nRet;
}

// 发送视频的sps和pps、vps信息
int SendVideoSpsPpsVps(RTMP* r, unsigned char* pps, int pps_len, unsigned char* sps, int sps_len, unsigned char* vps,
                       int vps_len, uint32_t dts)
{
    char        tBuffer[RTMP_HEAD_SIZE + 1024] = { 0 };
    RTMPPacket* packet = ( RTMPPacket* )tBuffer;
    packet->m_body = ( char* )packet + RTMP_HEAD_SIZE;
    unsigned char* body = ( unsigned char* )packet->m_body;
    // http://ffmpeg.org/doxygen/trunk/hevc_8c_source.html#l00040  hvcc_write 函数
    // 在nginx-rtmp中会跳过48位不去处理 我们在表示后面补0
    // skip tag header and configurationVersion(1 byte)
    int i = 0;
    body[i++] = 0x1C;
    i += 5;  // 5个字节的0x00

    // general_profile_idc 8bit
    body[i++] = sps[1];
    // general_profile_compatibility_flags 32 bit
    body[i++] = sps[2];
    body[i++] = sps[3];
    body[i++] = sps[4];
    body[i++] = sps[5];

    // 48 bit NUll nothing deal in rtmp
    body[i++] = sps[6];
    body[i++] = sps[7];
    body[i++] = sps[8];
    body[i++] = sps[9];
    body[i++] = sps[10];
    body[i++] = sps[11];

    // general_level_idc
    body[i++] = sps[12];

    // 48 bit NUll nothing deal in rtmp
    i += 6;

    // bit(16) avgFrameRate;
    i += 2;

    /* bit(2) constantFrameRate; */
    /* bit(3) numTemporalLayers; */
    /* bit(1) temporalIdNested; */
    i += 1;

    /* unsigned int(8) numOfArrays; 03 */
    body[i++] = 0x03;
    // vps 32
    body[i++] = 0x20;
    body[i++] = (1 >> 8) & 0xff;
    body[i++] = 1 & 0xff;
    body[i++] = (vps_len >> 8) & 0xff;
    body[i++] = ( vps_len )&0xff;
    memcpy(&body[i], vps, vps_len);
    i += vps_len;

    // sps
    body[i++] = 0x21;  // sps 33
    body[i++] = (1 >> 8) & 0xff;
    body[i++] = 1 & 0xff;
    body[i++] = (sps_len >> 8) & 0xff;
    body[i++] = sps_len & 0xff;
    memcpy(&body[i], sps, sps_len);
    i += sps_len;

    // pps
    body[i++] = 0x22;  // pps 34
    body[i++] = (1 >> 8) & 0xff;
    body[i++] = 1 & 0xff;
    body[i++] = (pps_len >> 8) & 0xff;
    body[i++] = ( pps_len )&0xff;
    memcpy(&body[i], pps, pps_len);
    i += pps_len;

    // body[i++] = 0x27; //sei 39
    // writeByte_Short(body, 1);
    // body[i++] = (sei_len >> 8) & 0xff;
    // body[i++] = (sei_len) & 0xff;
    // memcpy(&body[i], sei, sei_len);
    // i += sei_len;

    packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    packet->m_nBodySize = i;
    packet->m_nChannel = 0x04;
    packet->m_nTimeStamp = dts;
    packet->m_hasAbsTimestamp = 0;
    packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
    packet->m_nInfoField2 = r->m_stream_id;

    int nRet = 0;
    if (1 == RTMP_IsConnected(r))
        nRet = RTMP_SendPacket(r, packet, TRUE);  // 1为放进发送队列,0是不放进发送队列,直接发送
    return nRet;
}

// 发送RTMP包
int SendPacket(RTMP* r, unsigned int nPacketType, unsigned char* data, unsigned int size, uint32_t nTimestamp)
{
    /*分配包内存和初始化,len为包体长度*/
    char        tBuffer[RTMP_HEAD_SIZE + size] = { 0 };
    RTMPPacket* packet = ( RTMPPacket* )tBuffer;
    /*包体内存*/
    packet->m_body = ( char* )packet + RTMP_HEAD_SIZE;
    packet->m_nBodySize = size;
    memcpy(packet->m_body, data, size);
    packet->m_hasAbsTimestamp = 0;
    packet->m_packetType = nPacketType; /*此处为类型有两种一种是音频,一种是视频*/
    packet->m_nInfoField2 = r->m_stream_id;
    packet->m_nChannel = 0x04;

    packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
    if (RTMP_PACKET_TYPE_AUDIO == nPacketType) {
        packet->m_nChannel = 0x05;
        packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
    }
    packet->m_nTimeStamp = nTimestamp;
    /*发送*/
    int nRet = 0;
    if (1 == RTMP_IsConnected(r)) {
        nRet = RTMP_SendPacket(r, packet, FALSE); /*TRUE为放进发送队列,FALSE是不放进发送队列,直接发送*/
    }
    return nRet;
}

// H264 RTMP
int Send264Videoframe(RTMP* r, std::string strNalu, uint32_t dts)
{
    unsigned char*      frameBuf = ( unsigned char* )strNalu.c_str();
    int                 frameLen = strNalu.length();
    int                 nut = frameBuf[0] & 0x1F;
    static AvcExtData_t avcExt;
    switch (nut) {
    case avc::NALU_TYPE_SPS:
        avcExt.sps = strNalu;
        break;
    case avc::NALU_TYPE_PPS:
        avcExt.pps = strNalu;
        break;
    case avc::NALU_TYPE_SLICE:
    case avc::NALU_TYPE_IDR: {
        if (avcExt.sps.empty() || avcExt.pps.empty()) {
            break;
        }
        unsigned char body[frameLen + 9] = { 0 };
        int           i = 0;
        int           bRet = 0;
        if (nut == avc::NALU_TYPE_IDR) {
            body[i++] = 0x17;  // 1:Iframe  7:AVC
            if (SendVideoSpsPps(r, ( unsigned char* )avcExt.pps.c_str(), avcExt.pps.length(),
                                ( unsigned char* )avcExt.sps.c_str(), avcExt.sps.length(), dts)
                == 0) {
                break;
            }
        } else {
            body[i++] = 0x27;  // 2:Pframe  7:AVC
        }
        body[i++] = 0x01;  // AVC NALU
        i += 3;            // 3个字节的0x00
        // NALU size
        body[i++] = frameLen >> 24 & 0xff;
        body[i++] = frameLen >> 16 & 0xff;
        body[i++] = frameLen >> 8 & 0xff;
        body[i++] = frameLen & 0xff;
        // NALU data
        memcpy(&body[i], frameBuf, frameLen);
        return SendPacket(r, RTMP_PACKET_TYPE_VIDEO, body, i + frameLen, dts);
    } break;
    default:
        break;
    }
    return nut;
}

// H265 RTMP
int Send265Videoframe(RTMP* r, std::string strNalu, uint32_t dts)
{
    unsigned char*      frameBuf = ( unsigned char* )strNalu.c_str();
    int                 frameLen = strNalu.length();
    int                 nut = (frameBuf[0] & 0x7E) >> 1;
    static AvcExtData_t avcExt;
    switch (nut) {
    case hevc::NAL_UNIT_SPS:
        avcExt.sps = strNalu;
        break;
    case hevc::NAL_UNIT_PPS:
        avcExt.pps = strNalu;
        break;
    case hevc::NAL_UNIT_VPS:
        avcExt.vps = strNalu;
        break;
    case hevc::NAL_UNIT_CODED_SLICE_TRAIL_R:
    case hevc::NAL_UNIT_CODED_SLICE_IDR: {
        if (avcExt.sps.empty() || avcExt.pps.empty() || avcExt.vps.empty()) {
            break;
        }
        unsigned char body[frameLen + 9] = { 0 };
        int           i = 0;
        int           bRet = 0;
        if (nut == hevc::NAL_UNIT_CODED_SLICE_IDR) {
            body[i++] = 0x1C;  // 1:Iframe  7:AVC
            if (SendVideoSpsPpsVps(r, ( unsigned char* )avcExt.pps.c_str(), avcExt.pps.length(),
                                   ( unsigned char* )avcExt.sps.c_str(), avcExt.sps.length(),
                                   ( unsigned char* )avcExt.vps.c_str(), avcExt.vps.length(), dts)
                == 0) {
                break;
            }
        } else {
            body[i++] = 0x2C;  // 2:Pframe  7:AVC
        }
        body[i++] = 0x01;  // AVC NALU
        i += 3;            // 3个字节的0x00
        // NALU size
        body[i++] = frameLen >> 24 & 0xff;
        body[i++] = frameLen >> 16 & 0xff;
        body[i++] = frameLen >> 8 & 0xff;
        body[i++] = frameLen & 0xff;
        // NALU data
        memcpy(&body[i], frameBuf, frameLen);
        return SendPacket(r, RTMP_PACKET_TYPE_VIDEO, body, i + frameLen, dts);
    } break;
    default:
        break;
    }
    return nut;
}

// 发送AAC状态
int SendAccAudioframe(RTMP* r, unsigned char* data, int len, uint32_t dts, bool isAudio = true)
{
    unsigned char body[len + 2] = { 0 };
    int           i = 0;
    body[i++] = 0xAF;
    if (isAudio)
        body[i++] = 0x01;
    else
        body[i++] = 0x00;
    memcpy(&body[i], data, len);
    SendPacket(r, RTMP_PACKET_TYPE_AUDIO, body, i + len, dts);
}

#endif