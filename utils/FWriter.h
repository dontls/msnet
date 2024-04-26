#ifndef __FP_WRITER_H__
#define __FP_WRITER_H__

#include <stdio.h>

class FWriter {
 private:
  FILE* _fp;

 public:
  FWriter() {}
  ~FWriter() { fclose(_fp); }

  void initfp(const char* fpName) { _fp = fopen(fpName, "wb"); }

  size_t write(char* data, int len) {
    if (NULL == _fp) {
      return 0;
    }
    return fwrite(data, len, 1, _fp);
  }
};

#endif