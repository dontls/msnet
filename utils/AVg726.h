#ifndef __AV_G726_H__
#define __AV_G726_H__

#include "AVfaac.h"
#include "g7xx/g726.h"

const int KFlagHisiLen = 4;

class AVg726 {
private:
    g726_state_t* _stat;
    int           _nDecLen = 0;

public:
    AVg726(/* args */) {
        _stat = NULL;
    }
    ~AVg726() {}

    //  16000 24000 32000 40000
    //  G726_PACKING_NONE
    void init(int bitRate, int packing)
    {
        _stat = g726_init(NULL, bitRate, G726_ENCODING_LINEAR, packing);
        switch (bitRate) {
        case 16000:
            _nDecLen = 40;
            break;
        case 24000:
            _nDecLen = 60;
            break;
        case 32000:
            _nDecLen = 80;
            break;
        case 40000:
            _nDecLen = 100;
            break;
        default:
            break;
        }
    }

    void uninit()
    {
        if (_stat) {
            g726_release(_stat);
        }
    }

    // HIG726 转AMP,
    // 返回aac数据长度，0 失败
    int decodec(char* pSrcData, int nDataLen, char* pAmpData)
    {
        if (!_stat) {
            return 0;
        }
        // 去掉海思四个字节头
        char* g726Data = pSrcData + KFlagHisiLen;
        int   ampLen = g726_decode(_stat, ( int16_t* )pAmpData, ( const uint8_t* )g726Data, nDataLen - KFlagHisiLen);
        return ampLen * sizeof(int16_t);
    }

    int decodec2(char* pInBuf, int nInLen, char* pOutBuf)
    {
        int nOffset = 0;
        int nAudioFrameLen = 0;
        int nWriteOffset = 0;
        while (nOffset < nInLen) {
            nAudioFrameLen = 2 * pInBuf[2 + nOffset] + 4;
            if ((nAudioFrameLen + nOffset) > nInLen) {
                break;
            }

            int nDecOffset = 4;
            while (nDecOffset < nAudioFrameLen) {
                g726_decode(_stat, ( int16_t* )(pOutBuf + nWriteOffset),
                            ( const uint8_t* )(pInBuf + nOffset + nDecOffset), _nDecLen);
                nWriteOffset += 320;
                nDecOffset += _nDecLen;
            }
            nOffset += nAudioFrameLen;
        }
        return nWriteOffset;
    }

    // AMP编码，并添加海思头信息
    int encodec(char* pAmpData, int nDataLen, char* pG726Data)
    {
        if (!_stat) {
            return 0;
        }
        // 添加海思标识
        pG726Data[0] = 0;
        pG726Data[1] = 0x01;

        char* g726Data = pG726Data + KFlagHisiLen;
        int   g726Len = g726_encode(_stat, ( uint8_t* )g726Data, ( int16_t* )pAmpData, nDataLen / (sizeof(int16_t)));

        // 海思头长度按两个字节计算
        pG726Data[2] = g726Len / sizeof(int16_t);
        pG726Data[3] = 0;
        return g726Len + KFlagHisiLen;
    }
};

#endif