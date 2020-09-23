#ifndef __LLOC_BYTES_H__
#define __LLOC_BYTES_H__

#include <assert.h>

class LlocBytes {
private:
    char* _bytes;
    int   _maxBytesLen;

public:
    LlocBytes(/* args */)
    {
        _maxBytesLen = 40960;
         _bytes = new char[_maxBytesLen];
    }
    ~LlocBytes()
    {
        if (_bytes) {
            delete[] _bytes;
            _bytes = nullptr;
        }
    }
    
    // 申请空间
    char* Newlloc(int len)
    {
        if (_bytes == nullptr) {
            _bytes = new char[len];
            _maxBytesLen = len;
        }
        if (len > _maxBytesLen) {
            delete[] _bytes;
            _bytes = nullptr;
            _bytes = new char[len];
            _maxBytesLen = len;
        }
        assert(_bytes != nullptr);
        memset(_bytes, 0, _maxBytesLen);
        return _bytes;
    }
};

#endif