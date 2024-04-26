#include "FlvWriter.h"
#include <mutex>

FlvWriter::FlvWriter(/* args */) {}
FlvWriter::~FlvWriter() {}

bool FlvWriter::addFlvWriterConn(FlvConn_Ptr ptr) {
  std::lock_guard<std::mutex> lock(_mtx);
  if (_flvConns.find(ptr) == _flvConns.end()) {
    _flvConns.insert(ptr);
  }
  return true;
}

bool FlvWriter::delFlvWriterConn(FlvConn_Ptr ptr) {
  std::lock_guard<std::mutex> lock(_mtx);
  try {
    if (_flvConns.find(ptr) != _flvConns.end()) {
      _flvConns.erase(ptr);
    }
  } catch (...) {
  }
  return true;
}

bool FlvWriter::clearFlvWriterConn() {
  std::lock_guard<std::mutex> lock(_mtx);
  _flvConns.clear();
  return true;
}

// tagType audio0x08/video0x09
void FlvWriter::setFlvPacket(uint8_t tagType, std::string& flvTag,
                             unsigned long long apts, bool isMeta) {
  std::string flvPacket =
      _flvMuxer.flvPacket(tagType, flvTag.c_str(), flvTag.length(), apts);
  std::lock_guard<std::mutex> lock(_mtx);
  for (auto c : _flvConns)
    c->rawWriteFlvPacket(flvPacket.c_str(), flvPacket.length(),
                         _aacSpec.c_str(), _aacSpec.length(), isMeta);
}

void FlvWriter::setSpecificConfig(std::string aacSpec) {
  _aacSpec = _flvMuxer.flvPacket(0x08, aacSpec.c_str(), aacSpec.length(), 0);
}

FlvWriterManager* FlvWriterManager::ins() {
  static FlvWriterManager ins;
  return &ins;
}

FlvWriter_Ptr FlvWriterManager::flvWriter(std::string sessionId) {
  std::lock_guard<std::mutex> lock(_mtx);
  auto it = _flvWriterManager.find(sessionId);
  if (it != _flvWriterManager.end()) {
    return it->second;
  }
  FlvWriter_Ptr writer = std::make_shared<FlvWriter>();
  _flvWriterManager.insert(
      std::pair<std::string, FlvWriter_Ptr>(sessionId, writer));
  return writer;
}

void FlvWriterManager::delFlvWriter(std::string sessionId) {
  std::lock_guard<std::mutex> lock(_mtx);
  auto it = _flvWriterManager.find(sessionId);
  if (it != _flvWriterManager.end()) {
    it->second->clearFlvWriterConn();
  }
}