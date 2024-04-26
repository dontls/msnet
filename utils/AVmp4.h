#ifndef __AV_MP4_H__
#define __AV_MP4_H__

extern "C" {
#include <libavformat/avformat.h>
}

#include "AVg726ToAac.h"
#include "sps.hpp"
#include <string>

class AVmp4 {
 private:
  AVFormatContext* _pOfmtCtx;
  std::string _pfpName;
  int _nWidth, _nHeight, _nfps;

  uint32_t _nVideoStreamIndex;
  uint32_t _nAudioStreamIndex;

  // 扩展信息
  std::string _vExtraData;  // 视频

  unsigned long long _startVTime;
  bool _isWaitKeyframe;
  enum AVCodecID _nframeType;

  AVg726ToAac _g726ToAac;

 public:
  AVmp4(/* args */) {
    _pOfmtCtx = NULL;
    _nfps = 0;
    _isWaitKeyframe = true;
    _nframeType = AV_CODEC_ID_NONE;
    _g726ToAac.init(AAC_STREAM_ADTS);
  }
  ~AVmp4() {
    if (NULL != _pOfmtCtx) {
      av_write_trailer(_pOfmtCtx);
      avio_closep(&_pOfmtCtx->pb);
      avformat_free_context(_pOfmtCtx);
      _pOfmtCtx = NULL;
    }
    _g726ToAac.uinit();
  }

  void fpname(const char* fp) {
    _pfpName = fp;
    if (!strstr(fp, ".mp4")) {
      _pfpName.append(".mp4");
    }
  }

  void writeVideoframe(char* frame, int len, int type, unsigned long long pts) {
    // 关键帧
    if (_isWaitKeyframe && type == 0x01) {
      // 解析视频数据格式
      unsigned char type = frame[4] & 0x1F;
      // 0x01 slice 0x05 idr 0x06 sei 0x07 sps 0x08 pps
      switch (type) {
        case avc::NALU_TYPE_IDR:
        case avc::NALU_TYPE_SPS:
        case avc::NALU_TYPE_PPS:
        case avc::NALU_TYPE_SLICE:
        case avc::NALU_TYPE_SEI:
          _nframeType = AV_CODEC_ID_H264;
          break;
        default:
          _nframeType = AV_CODEC_ID_HEVC;
          break;
      }
      if (doParseExtraData(frame, len)) {
        // 写入扩展信息
        if (initfp()) {
          _isWaitKeyframe = false;
          _startVTime = pts;
        }
      }
    }
    if (_nframeType == AV_CODEC_ID_NONE || _isWaitKeyframe) {
      return;
    }
    uint32_t dts = (pts - _startVTime) / 1000;
    AVPacket pkt;
    av_init_packet(&pkt);
    if (type == 0x01) {
      pkt.flags |= AV_PKT_FLAG_KEY;
    }
    pkt.stream_index = _nVideoStreamIndex;
    pkt.data = (uint8_t*)frame;
    pkt.size = (int)len;

    // 当前相对于第一包数据的毫秒数
    pkt.dts = dts;
    pkt.pts = dts;

    //_pOfmtCtx里面的time_base 和
    //_pIfmtCtx里面的time_base不一样，一定要做下面的转换，否则，就会出现视频时长不正确，帧率不一样的错误
    av_packet_rescale_ts(&pkt, av_make_q(1, 1000),
                         _pOfmtCtx->streams[pkt.stream_index]->time_base);
    int ret = av_interleaved_write_frame(_pOfmtCtx, &pkt);
    avio_flush(_pOfmtCtx->pb);
    av_packet_unref(&pkt);
  }

  void writeAudioframe(char* frame, int len, unsigned long long pts) {
    if (_isWaitKeyframe) {  // 等待主帧
      return;
    }

    // 当前时间戳比第一包时间戳早,则不处理
    int aacLen = 0;
    unsigned char* aac = _g726ToAac.toAacEncodec(frame, len, aacLen);
    if (aacLen == 0) {
      return;
    }
    uint32_t dts = (pts - _startVTime) / 1000;
    AVPacket pkt;
    av_init_packet(&pkt);

    pkt.stream_index = _nAudioStreamIndex;
    pkt.data = (uint8_t*)aac;
    pkt.size = (int)aacLen;

    // 当前相对于第一包数据的毫秒数
    pkt.dts = dts;
    pkt.pts = dts;

    //_pOfmtCtx里面的time_base 和
    //_pIfmtCtx里面的time_base不一样，一定要做下面的转换，否则，就会出现视频时长不正确，帧率不一样的错误
    av_packet_rescale_ts(&pkt, av_make_q(1, 1000),
                         _pOfmtCtx->streams[pkt.stream_index]->time_base);
    int ret = av_interleaved_write_frame(_pOfmtCtx, &pkt);
    avio_flush(_pOfmtCtx->pb);
    av_packet_unref(&pkt);
  }

 private:
  bool initfp() {
    av_register_all();
    int code = avformat_alloc_output_context2(&_pOfmtCtx, NULL, NULL,
                                              _pfpName.c_str());
    if (code != 0 || NULL == _pOfmtCtx) {
      return false;
    }
    _pOfmtCtx->debug |= FF_FDEBUG_TS;

    code = avio_open(&(_pOfmtCtx->pb), _pfpName.c_str(), AVIO_FLAG_WRITE);
    if (0 != code) {
      return false;
    }
    newAVStream();
    // 写入流信息
    avformat_write_header(_pOfmtCtx, NULL);
    return true;
  }

  // H264sps信息解析
  void doAvcExtraData(std::string strNalu) {
    unsigned char* frameBuf = (unsigned char*)strNalu.c_str();
    int frameLen = strNalu.length();
    int nut = frameBuf[0] & 0x1F;
    const char nalucode[4] = {0x00, 0x00, 0x00, 0x01};
    switch (nut) {
      case avc::NALU_TYPE_SPS:
        avc::decode_sps(frameBuf, frameLen, _nWidth, _nHeight, _nfps);
      case avc::NALU_TYPE_PPS:
        _vExtraData.append(std::string(nalucode, 4));
        _vExtraData.append(strNalu);
      case avc::NALU_TYPE_IDR:
      case avc::NALU_TYPE_SLICE:
      case avc::NALU_TYPE_SEI:
        break;
      default:
        break;
    }
  }

  // H265sps信息解析
  void doHevcExtraData(std::string strNalu) {
    unsigned char* frameBuf = (unsigned char*)strNalu.c_str();
    int frameLen = strNalu.length();
    int nut = (frameBuf[0] & 0x7E) >> 1;
    const char nalucode[5] = {0x00, 0x00, 0x00, 0x01};
    switch (nut) {
      case hevc::NAL_UNIT_SPS:
        hevc::decode_sps(frameBuf, frameLen, _nWidth, _nHeight, _nfps);
      case hevc::NAL_UNIT_PPS:
      case hevc::NAL_UNIT_VPS:
        _vExtraData.append(nalucode, 4);
        _vExtraData.append(strNalu);
        break;
      case hevc::NAL_UNIT_CODED_SLICE_TRAIL_R:
      case hevc::NAL_UNIT_CODED_SLICE_IDR:
        break;
      default:
        break;
    }
  }

  bool doParseExtraData(char* frame, int len) {
    std::vector<std::string> naluVec = ParseNalUnit(frame, len);
    int naluSize = naluVec.size();
    if (naluSize < 0) {
      return false;
    }
    for (int i = 0; i < naluSize; i++) {
      switch (_nframeType) {
        case AV_CODEC_ID_H264:
          doAvcExtraData(naluVec[i]);
          break;
        case AV_CODEC_ID_HEVC:
          doHevcExtraData(naluVec[i]);
          break;
        default:
          break;
      }
    }
    return true;
  }

  void newAVStream() {
    {
      AVStream* pAvStream = avformat_new_stream(_pOfmtCtx, NULL);
      if (pAvStream == NULL) {
        return;
      }
      _nVideoStreamIndex = _pOfmtCtx->nb_streams - 1;
      pAvStream->codecpar->width = _nWidth;
      pAvStream->codecpar->height = _nHeight;
      pAvStream->codecpar->format = AV_PIX_FMT_YUV420P;
      pAvStream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
      pAvStream->codecpar->codec_id = _nframeType;
      pAvStream->avg_frame_rate = av_make_q(_nfps, 1);
      pAvStream->r_frame_rate = av_make_q(_nfps, 1);

      // 此处需要按照该规则来创建对应的缓冲区,否则调用avformat_free_context会报错
      // 也可以采用自己的缓冲区,然后再avformat_free_context之前把这两个变量置空
      //  需要把Sps/Pps/Vps写入扩展信息中
      int videoExtraLength = _vExtraData.length();
      pAvStream->codecpar->extradata =
          (uint8_t*)av_malloc(videoExtraLength + AV_INPUT_BUFFER_PADDING_SIZE);
      ::memcpy(pAvStream->codecpar->extradata, _vExtraData.c_str(),
               videoExtraLength);
      pAvStream->codecpar->extradata_size = videoExtraLength;
    }
    //  创建音频输入格式
    {
      AVStream* pAvStream = avformat_new_stream(_pOfmtCtx, NULL);
      if (pAvStream == NULL) {
        return;
      }
      _nAudioStreamIndex = _pOfmtCtx->nb_streams - 1;

      pAvStream->codecpar->channels = 1;
      pAvStream->codecpar->channel_layout =
          av_get_default_channel_layout(pAvStream->codecpar->channels);
      pAvStream->codecpar->sample_rate = 8000;
      pAvStream->codecpar->format = AV_SAMPLE_FMT_S16;
      pAvStream->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
      pAvStream->codecpar->codec_id = AV_CODEC_ID_AAC;
      pAvStream->codecpar->frame_size = 1024;

      pAvStream->codecpar->bit_rate = 16000;

      int specialLen = 0;
      unsigned char* pSpecialData = _g726ToAac.aacSpecialData(&specialLen);

      pAvStream->codecpar->extradata =
          (uint8_t*)av_malloc(specialLen + AV_INPUT_BUFFER_PADDING_SIZE);
      ::memcpy(pAvStream->codecpar->extradata, pSpecialData, specialLen);
      pAvStream->codecpar->extradata_size = specialLen;
    }
  }
};

#endif
