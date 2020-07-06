#ifndef __AV_G716TOAAC_H__
#define __AV_G716TOAAC_H__

#include "AVfaac.h"
#include "AVg726.h"

const int KAmpBufSize = 2048;

class AVg726ToAac {
private:
    AVg726* _avg726;
    AVfaac* _avfaac;
    char*   _pAmpBuffer;
    int     _nAmpOffset;

public:
    AVg726ToAac(/* args */) : _avg726(new AVg726), _avfaac(new AVfaac)
    {
        _pAmpBuffer = NULL;
    }
    ~AVg726ToAac() {}

    void init(int nOfmt = AAC_STREAM_RAW)
    {
        _avg726->init(40000, G726_PACKING_RIGHT);
        _avfaac->init(8000, 1, 16, nOfmt);
        _pAmpBuffer = new char[KAmpBufSize * 2];
        _nAmpOffset = 0;
    }

    void uinit()
    {
        _avg726->uninit();
        if (_avg726) {
            delete _avg726;
            _avg726 = nullptr;
        }
        _avfaac->uninit();

        if (_avfaac) {
            delete _avfaac;
            _avfaac = nullptr;
        }
        if (_pAmpBuffer) {
            delete[] _pAmpBuffer;
            _pAmpBuffer = NULL;
        }
    }

    unsigned char* aacSpecialData(int* len)
    {
        return _avfaac->specialData(len);
    }

    unsigned char* toAacEncodec(char* data, int len, int& aacLen)
    {
        aacLen = 0;
        // 海思G726转PCM
        int   reLen = KAmpBufSize - _nAmpOffset;
        char* offsetAmpBuf = _pAmpBuffer + _nAmpOffset;
        int   pcmLen = _avg726->decodec2(data, len, offsetAmpBuf);
        if (pcmLen != 640) {
            return NULL;
        }

        // 数据不够
        _nAmpOffset += pcmLen;
        if (_nAmpOffset < KAmpBufSize) {
            return NULL;
        }

        // PCM编码AAC
        unsigned char* aac = _avfaac->encode(( unsigned char* )_pAmpBuffer, KAmpBufSize, aacLen);
        if (aacLen == 0) {
            return NULL;
        }
        // 需要更新AMP Buffer数据，并且删除原有的数据
        int freeAmpLen = _nAmpOffset - KAmpBufSize;  // 剩余未编码AAC的PCM数据
        if (freeAmpLen > 0) {
            ::memmove(_pAmpBuffer, _pAmpBuffer + KAmpBufSize, freeAmpLen);
        }
        _nAmpOffset = freeAmpLen;
        return aac;
    }
};

#endif