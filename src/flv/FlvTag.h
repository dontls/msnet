#ifndef FLVTAG_H
#define FLVTAG_H

#include "buffer.hpp"
#include <stdio.h>
#include <string.h>
#include <string>

class FlvTag {
 private:
  libyte::Buffer _lBytes;

 public:
  FlvTag(/* args */) {}
  ~FlvTag() {}

  // i/p frame
  std::string flvVideoTag(const char* raw, int len, bool isKey,
                          bool isH265 = false) {
    _lBytes.Realloc(len + 9);
    uint8_t* output = (uint8_t*)_lBytes.Bytes();
    int i = 0;
    // flv VideoTagHeader
    if (isKey) {
      output[i++] = isH265 ? 0x1C : 0x17;  // key frame, AVC
    } else {
      output[i++] = isH265 ? 0x2C : 0x27;  // p frame, AVC
    }
    output[i++] = 0x01;  // avc NALU unit
    i += 3;
    output[i++] = (uint8_t)(len >> 24);  // nal length
    output[i++] = (uint8_t)(len >> 16);  // nal length
    output[i++] = (uint8_t)(len >> 8);   // nal length
    output[i++] = (uint8_t)(len);        // nal length
    memcpy(output + i, raw, len);
    i += len;
    return std::string((char*)output, i);
  }

  // sps/pps
  std::string flvMetaTag(const char* sps, int spsLen, const char* pps,
                         int ppsLen) {
    _lBytes.Realloc(spsLen + ppsLen + 16);
    uint8_t* output = (uint8_t*)_lBytes.Bytes();
    int i = 0;
    // flv VideoTagHeader
    output[i++] = 0x17;  // key frame, AVC
    i += 4;
    // flv VideoTagBody --AVCDecoderCOnfigurationRecord
    output[i++] = 0x01;    // configurationversion
    output[i++] = sps[1];  // avcprofileindication
    output[i++] = sps[2];  // profilecompatibilty
    output[i++] = sps[3];  // avclevelindication
    output[i++] = 0xff;    // reserved + lengthsizeminusone

    output[i++] = 0xe1;  // numofsequenceset
    output[i++] =
        (uint8_t)(spsLen >> 8);  // sequence parameter set length high 8 bits
    output[i++] =
        (uint8_t)(spsLen);  // sequence parameter set  length low 8 bits
    memcpy(output + i, sps, spsLen);  // H264 sequence parameter set
    i += spsLen;
    output[i++] = 0x01;  // numofpictureset
    output[i++] =
        (uint8_t)(ppsLen >> 8);  // picture parameter set length high 8 bits
    output[i++] = (uint8_t)(ppsLen);  // picture parameter set length low 8 bits
    memcpy(output + i, pps, ppsLen);  // H264 picture parameter set
    i += ppsLen;
    return std::string((char*)output, i);
  }

  // sps/pps
  std::string flvHevcMetaTag(const char* vps, int vpsLen, const char* sps,
                             int spsLen, const char* pps, int ppsLen) {
    _lBytes.Realloc(vpsLen + spsLen + ppsLen + 43);
    uint8_t* output = (uint8_t*)_lBytes.Bytes();
    int i = 0;
    // flv VideoTagHeader
    output[i++] = 0x1C;  // key frame, AVC
    i += 5;
    // general_profile_idc 8bit
    output[i++] = sps[1];
    // general_profile_compatibility_flags 32 bit
    output[i++] = sps[2];
    output[i++] = sps[3];
    output[i++] = sps[4];
    output[i++] = sps[5];

    // 48 bit NUll nothing deal in rtmp
    output[i++] = sps[6];
    output[i++] = sps[7];
    output[i++] = sps[8];
    output[i++] = sps[9];
    output[i++] = sps[10];
    output[i++] = sps[11];
    // general_level_idc
    output[i++] = sps[12];
    // 48 bit NUll nothing deal in rtmp
    i += 6;

    // bit(16) avgFrameRate;
    i += 2;

    /* bit(2) constantFrameRate; */
    /* bit(3) numTemporalLayers; */
    /* bit(1) temporalIdNested; */
    output[i++] = 0x00;
    /* unsigned int(8) numOfArrays; 03 */
    output[i++] = 0x03;
    // vps 32
    output[i++] = 0x20;
    output[i++] = (1 >> 8) & 0xff;
    output[i++] = 1 & 0xff;
    output[i++] = (vpsLen >> 8) & 0xff;
    output[i++] = (vpsLen)&0xff;
    memcpy(&output[i], vps, vpsLen);
    i += vpsLen;

    // sps
    output[i++] = 0x21;  // sps 33
    output[i++] = (1 >> 8) & 0xff;
    output[i++] = 1 & 0xff;
    output[i++] = (spsLen >> 8) & 0xff;
    output[i++] = (spsLen)&0xff;
    memcpy(&output[i], sps, spsLen);
    i += spsLen;

    // pps
    output[i++] = 0x22;  // pps 34
    output[i++] = (1 >> 8) & 0xff;
    output[i++] = 1 & 0xff;
    output[i++] = (ppsLen >> 8) & 0xff;
    output[i++] = (ppsLen)&0xff;
    memcpy(&output[i], pps, ppsLen);
    i += ppsLen;
    return std::string((char*)output, i);
  }

  // audio
  std::string flvAudioTag(unsigned char* raw, int len, bool isSpec = false) {
    _lBytes.Realloc(len + 2);
    uint8_t* output = (uint8_t*)_lBytes.Bytes();
    int i = 0;
    output[i++] = 0xAF;
    if (isSpec) {
      output[i++] = 0x00;
    } else {
      output[i++] = 0x01;
    }
    memcpy(output + i, raw, len);
    i += len;
    return std::string((char*)output, i);
  }
};

#endif