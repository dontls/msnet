#include "RtmpWriter.h"
#include "Conf.h"
#include "Log.h"
#include "UtilRtmp.h"

#ifdef SRS_LIBRTMP
// 这里只能上传到srs服务，nginx不行
void ssrPublishVideoframe(srs_rtmp_t rtmp, char* frame, int len, int type, unsigned long long pts)
{
    if (_startVTime == 0) {
        _startVTime = pts;
    }
    uint32_t dts = (pts - _startVTime) / 1000;
    _errCode = srs_h264_write_raw_frames(_rtmp, frame, len, dts, dts);
    if (srs_h264_is_dvbsp_error(_errCode)) {
        _errMsg = "ignore drop video error";
    } else if (srs_h264_is_duplicated_sps_error(_errCode)) {
        _errMsg = "ignore duplicated sps";
    } else if (srs_h264_is_duplicated_pps_error(_errCode)) {
        _errMsg = "ignore duplicated pps";
    } else {
        _errMsg = "send h264 raw data failed";
    }
}

// G726 转 pcm
void ssrPublishAudioframe(srs_rtmp_t rtmp, char* frame, int len, unsigned long long pts)
{
    // 直接发送pcm
    srs_audio_write_raw_frame(_rtmp, 3, 3, 1, 0, ampBuf, pcmLen, (pts - _startVTime) / 1000);
    return;
    // _fpWriter.write(frame, len);
    // // 发送AAC
    // if (_startATime == 0) {
    //     _startATime = pts;
    //     _g726ToAac.init();
    // }
    // int            aacLen = 0;
    // unsigned char* aac = _g726ToAac.toAacEncodec(frame, len, aacLen);
    // if (aacLen == 0) {
    //     return;
    // }
    // uint32_t dts = (pts - _startVTime) / 1000;
    // srs_audio_write_raw_frame(_rtmp, 10, 1, 1, 1, ( char* )aac, aacLen, dts);
}
#endif
RtmpWriter::RtmpWriter(/* args */) {}

RtmpWriter::~RtmpWriter()
{
#ifdef SRS_LIBRTMP
    srs_rtmp_destroy(_rtmp);
#else
    if (_rtmp) {
        RTMP_Close(_rtmp);
        RTMP_Free(_rtmp);
        _rtmp = NULL;
    }
#endif
}

bool RtmpWriter::init(const char* session, int frameType)
{
    std::string rtmpUrl = GetRtmpBaseUrl();
    if (!initUrl(rtmpUrl.append(session).c_str())) {
        return false;
    }
    if (frameType == 265) {
        _publishFunc = Send265Videoframe;
    } else {
        _publishFunc = Send264Videoframe;
    }
    return true;
}

void RtmpWriter::writeVideoFrame(std::string strNalu, uint32_t dts)
{
#ifdef SRS_LIBRTMP
#else
    _publishFunc(_rtmp, strNalu, dts);
#endif
}

void RtmpWriter::writeAudioFrame(unsigned char* data, int len, uint32_t dts, bool isAudio)
{
#ifdef SRS_LIBRTMP
#else
    SendAccAudioframe(_rtmp, data, len, dts, isAudio);
#endif
}

bool RtmpWriter::initUrl(const char* url)
{
    LOG("%s\n", url);
    do {
#if defined(SRS_LIBRTMP)
        _rtmp = srs_rtmp_create(url);
        if (srs_rtmp_handshake(_rtmp) != 0) {
            _errMsg = "simple handshake failed.";
            break;
        }
        if (srs_rtmp_connect_app(_rtmp) != 0) {
            _errMsg = "connect vhost/app failed.";
            break;
        }
        if (srs_rtmp_publish_stream(_rtmp) != 0) {
            _errMsg = "publish stream failed.";
            break;
        }
#else
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
#endif
        return true;
    } while (0);
    LOG("%s\n", _errMsg);
    return false;
}
