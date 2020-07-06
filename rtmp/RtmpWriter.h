#ifndef RTMP_PUBLISH_H
#define RTMP_PUBLISH_H

#ifdef SRS_LIBRTMP
#include "srs/srs_librtmp.h"
#else
#include "librtmp/log.h"
#include "rtmp.h"
#endif
#include <functional>


class RtmpWriter {
private:
    const char* _errMsg;
    // 发布流
#ifdef SRS_LIBRTMP
    srs_rtmp_t _rtmp;
#else
    RTMP*                                            _rtmp;
    std::function<int(RTMP*, std::string, uint32_t)> _publishFunc;
#endif
public:
    RtmpWriter(/* args */);
    ~RtmpWriter();

public:
    bool init(const char* session, int frameType);

    void writeVideoFrame(std::string strNalu, uint32_t dts);

    void writeAudioFrame(unsigned char* data, int len, uint32_t dts, bool isAudio = true);

private:
    bool initUrl(const char* url);
};
#endif