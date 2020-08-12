#ifndef __AV_FLV_H__
#define __AV_FLV_H__

#include "utils/LlocBytes.h"
#include <string.h>
#include <string>
// 只支持h264、aac
//
class FlvMuxer {
private:
    const int FLV_TagHeaderLen = 11;
    const int FLV_PreTagLen = 4;
    const int FLV_AacAdtsHeaderSize = 7;
    LlocBytes _lBytes;

public:
    FlvMuxer(/* args */) {}
    ~FlvMuxer() {}
    // video tagType 0x09
    // audio tagType 0x08
    std::string flvPacket(uint8_t tagType, const char* tag, int tagLen, uint32_t abstamp)
    {
        uint8_t* output = ( uint8_t* )_lBytes.Newlloc(tagLen + FLV_TagHeaderLen + FLV_PreTagLen);
        uint32_t i = 0;
        output[i++] = tagType;
        output[i++] = (uint8_t)(tagLen >> 16);   // data len
        output[i++] = (uint8_t)(tagLen >> 8);    // data len
        output[i++] = (uint8_t)(tagLen);         // data len
        output[i++] = (uint8_t)(abstamp >> 16);  // time stamp
        output[i++] = (uint8_t)(abstamp >> 8);   // time stamp
        output[i++] = (uint8_t)(abstamp);        // time stamp
        output[i++] = (uint8_t)(abstamp >> 24);  // time stamp
        output[i++] = 0x00;                      // stream id 0
        output[i++] = 0x00;                      // stream id 0
        output[i++] = 0x00;                      // stream id 0
        memcpy(output + i, tag, tagLen);
        i += tagLen;
        uint32_t fff = tagLen + FLV_TagHeaderLen;
        output[i++] = (uint8_t)(fff >> 24);  // data len
        output[i++] = (uint8_t)(fff >> 16);  // data len
        output[i++] = (uint8_t)(fff >> 8);   // data len
        output[i++] = (uint8_t)(fff);        // data len
        return std::string(( char* )output, i);
    }
};

#endif