#ifndef PUBLISHER_H
#define PUBLISHER_H

#include <memory>

#include "AVg726ToAac.h"
#include "flv/FlvTag.h"
#include "flv/FlvWriter.h"
#include "rtmp/RtmpFlvWriter.h"
// #include "AVmp4.h"

#include <string>
class Publisher {
private:
    typedef struct AvcExtData {
        std::string pps;
        std::string sps;
        std::string vps;
    } AvcExtData_t;

private:
    AVg726ToAac*       _g726ToAac;
    unsigned long long _startAVTime;
    bool               _isWaitKeyframe;
    int                _frameFormatType;
    std::string        _session;
    RtmpFlvWriter_Ptr  _rtmpFlvWriter;
    FlvWriter_Ptr      _flvWriter;
    FlvTag             _flvTag;
    AvcExtData_t       _avcExt;
    std::string        _aacFlvTagSpec;

public:
    Publisher();
    ~Publisher();
    bool initSession(std::string session);
    void recvVideoRaw(char* frame, int len, int type, unsigned long long pts);
    void recvAudioRaw(char* frame, int len, unsigned long long pts);

private:
    void publishAvcNaluRaw(std::string& naluRaw, uint32_t abstamp);
    void publishHevcNaluRaw(std::string& naluRaw, uint32_t abstamp);
    bool initWriter();
    void initAac();
    void frameFormat(char* frame);
};

typedef std::shared_ptr<Publisher> Publisher_Ptr;

#endif