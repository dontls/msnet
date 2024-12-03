#ifndef FLVWRITER_H
#define FLVWRITER_H

#include "FlvConn.h"
#include <mutex>
#include <map>
#include <set>
#include <string>
#include "buffer.hpp"

class FlvWriter {
 private:
  std::mutex _mtx;
  std::set<FlvConn_Ptr> _flvConns;
  std::string _aacSpec;  // 记录aac spec
 public:
  FlvWriter(/* args */);
  ~FlvWriter();

 public:
  bool addFlvWriterConn(FlvConn_Ptr ptr);
  bool delFlvWriterConn(FlvConn_Ptr ptr);
  bool clearFlvWriterConn();

  // tagType audio0x08/video0x09
  void WriteFrame(char* data, size_t n, bool bkey);
  void WriteAACSpec(std::string& s);
};

typedef std::shared_ptr<FlvWriter> FlvWriter_Ptr;

class FlvWriterManager {
  std::map<std::string, FlvWriter_Ptr> _flvWriterManager;
  std::mutex _mtx;

 public:
  static FlvWriterManager* ins();

  FlvWriter_Ptr flvWriter(std::string sessioId);
  void delFlvWriter(std::string sessioId);
};

#endif