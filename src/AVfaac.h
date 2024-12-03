#ifndef __AVFAAC_H__
#define __AVFAAC_H__

#include "faac.h"
#include <string.h>

enum {
  AAC_STREAM_RAW = 0,
  AAC_STREAM_ADTS = 1,
};

class AVfaac {
 private:
  faacEncHandle _faacEnc;
  unsigned long _maxOutputBytes;
  unsigned long _inputSamples;
  int _nBitPerSample;  // 采样本位数
  int _nSpecialLen;
  char _nDuration;  // 采样间隔ms
  unsigned char* _pOutputBytes;
  char _cSpecialData[8];  // 音频特殊信息

 public:
  AVfaac(/* args */) {
    _faacEnc = NULL;
    _pOutputBytes = NULL;
  }
  ~AVfaac() {}

  // 8000 1 16
  bool init(int nSamplePerSec, int nChn, int nBitsPerSample, int nOfmt) {
    _nBitPerSample = nBitsPerSample;
    _faacEnc =
        faacEncOpen(nSamplePerSec, nChn, &_inputSamples, &_maxOutputBytes);
    // unsigned long maxInputBytes =_inputSamples * nBitsPerSample / 8;  //
    // 计算最大输入字节
    faacEncConfigurationPtr pfaacEncConf =
        faacEncGetCurrentConfiguration(_faacEnc);
    pfaacEncConf->inputFormat = FAAC_INPUT_16BIT;
    pfaacEncConf->outputFormat = nOfmt;  // 0:RAW——STREAM	1:ADTS-STREAM
    pfaacEncConf->useTns = true;
    pfaacEncConf->useLfe = false;
    pfaacEncConf->aacObjectType = LOW;
    pfaacEncConf->shortctl = SHORTCTL_NORMAL;
    pfaacEncConf->quantqual = 100;
    pfaacEncConf->bandWidth = 0;
    pfaacEncConf->bitRate = 44100;
    pfaacEncConf->mpegVersion = MPEG2;
    pfaacEncConf->version = MPEG2;
    faacEncSetConfiguration(_faacEnc, pfaacEncConf);

    unsigned char* pConfBuffer = 0;
    unsigned long ulConfigLength = 0;
    faacEncGetDecoderSpecificInfo(_faacEnc, &pConfBuffer, &ulConfigLength);

    ::memcpy(_cSpecialData, pConfBuffer, ulConfigLength);
    _nSpecialLen = (int)ulConfigLength;

    // 音频帧的播放时间 = 一个AAC帧对应的采样样本的个数 / 采样频率(单位为s)
    // 一帧 1024个 sample。采样率 Samplerate 44100KHz，每秒44100个sample, 所以
    // 根据公式   音频帧的播放时间 = 一个AAC帧对应的采样样本的个数 / 采样频率
    // 当前AAC一帧的播放时间是 = 1024 * 1000000 / 44100 = 22320ns(单位为ns)
    _nDuration = 1024 * 1000 / nSamplePerSec;
    _pOutputBytes = new unsigned char[_maxOutputBytes];
    return true;
  }

  void uninit() {
    if (_faacEnc) {
      faacEncClose(_faacEnc);
    }
    if (_pOutputBytes) {
      delete[] _pOutputBytes;
      _pOutputBytes = NULL;
    }
  }

  // 编码 返回编码后数据长度
  char* encode(char* pcmData, long dataLen, int& accLen) {
    if (NULL == _faacEnc) {
      return 0;
    }
    int nInputSample = dataLen / (_nBitPerSample / 8);
    int nEncDataLen = 0;
    for (;;) {
      nEncDataLen = faacEncEncode(_faacEnc, (int*)pcmData, nInputSample,
                                  _pOutputBytes, _maxOutputBytes);
      if (nEncDataLen < 1) {
        continue;
      }
      break;
    }
    accLen = nEncDataLen;
    return (char*)_pOutputBytes;
  }

  // 播放时间周期
  int duration() { return _nDuration; }

  char* specialData(int* len) {
    *len = _nSpecialLen;
    return _cSpecialData;
  }
};

#endif