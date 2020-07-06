#ifndef __FLV_TAG_H__
#define __FLV_TAG_H__

#include <stdio.h>
#include <string.h>
#include <string>

class FlvTag {
private:
public:
    FlvTag(/* args */) {}
    ~FlvTag() {}

    // i/p frame
    std::string flvVideoTag(const char* raw, int len, bool isKey)
    {
        char output[len + 9] = { 0 };
        int  i = 0;
        // flv VideoTagHeader
        if (isKey) {
            output[i++] = 0x17;  // key frame, AVC
        } else {
            output[i++] = 0x27;  // p frame, AVC
        }
        output[i++] = 0x01;  // avc NALU unit
        i += 3;

        output[i++] = (uint8_t)(len >> 24);  // nal length
        output[i++] = (uint8_t)(len >> 16);  // nal length
        output[i++] = (uint8_t)(len >> 8);   // nal length
        output[i++] = (uint8_t)(len);        // nal length
        memcpy(output + i, raw, len);
        i += len;
        std::string ostr(output, i);
        return ostr;
    }

    // sps/pps
    std::string flvMetaTag(const char* sps, int spsLen, const char* pps, int ppsLen)
    {
        char output[spsLen + ppsLen + 16] = { 0 };
        int  i = 0;
        // flv VideoTagHeader
        output[i++] = 0x17;  // key frame, AVC
        i += 4;

        // flv VideoTagBody --AVCDecoderCOnfigurationRecord
        output[i++] = 0x01;    // configurationversion
        output[i++] = sps[1];  // avcprofileindication
        output[i++] = sps[2];  // profilecompatibilty
        output[i++] = sps[3];  // avclevelindication
        output[i++] = 0xff;    // reserved + lengthsizeminusone

        output[i++] = 0xe1;                    // numofsequenceset
        output[i++] = (uint8_t)(spsLen >> 8);  // sequence parameter set length high 8 bits
        output[i++] = (uint8_t)(spsLen);       // sequence parameter set  length low 8 bits
        memcpy(output + i, sps, spsLen);       // H264 sequence parameter set
        i += spsLen;
        output[i++] = 0x01;                    // numofpictureset
        output[i++] = (uint8_t)(ppsLen >> 8);  // picture parameter set length high 8 bits
        output[i++] = (uint8_t)(ppsLen);       // picture parameter set length low 8 bits
        memcpy(output + i, pps, ppsLen);       // H264 picture parameter set
        i += ppsLen;
        std::string ostr(output, i);
        return ostr;
    }

    // audio
    std::string flvAudioTag(unsigned char* raw, int len, bool isSpec = false)
    {
        char output[len + 2] = { 0 };
        int  i = 0;
        output[i++] = 0xAF;
        if (isSpec) {
            i++;
        } else {
            output[i++] = 0x01;
        }
        memcpy(output + i, raw, len);
        i += len;
        std::string ostr(output, i);
        return ostr;
    }
};

#endif