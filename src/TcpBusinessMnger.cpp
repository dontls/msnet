#include "TcpBusinessMnger.h"
#include "time.hpp"

TcpBusinessMnger::TcpBusinessMnger(/* args */) {}

TcpBusinessMnger::~TcpBusinessMnger() {}

TcpBusinessMnger* TcpBusinessMnger::ins() {
  static TcpBusinessMnger ins;
  return &ins;
}

bool TcpBusinessMnger::add(std::string sessionId, TcpBusiness_Ptr ptr) {
  std::lock_guard<std::mutex> lock(_mtx);
  auto it = _businessMap.find(sessionId);
  if (it != _businessMap.end()) {
    return false;
  }
  TcpBusiness_t b;
  b.tick = libtime::UnixMilli();
  b.business = ptr;
  _businessMap.insert(TyMapTcpBusiness::value_type(sessionId, b));
  return true;
}

void TcpBusinessMnger::remove(std::string sessionId, int nclient,
                              int ncloseTime) {
  std::lock_guard<std::mutex> lock(_mtx);
  auto it = _businessMap.find(sessionId);
  if (it == _businessMap.end()) {
    return;
  }
  TcpBusiness_t stc = it->second;
  // 存在消费者， 更新时间戳
  if (nclient > 1) {
    stc.tick = libtime::UnixMilli();
    return;
  }
  // 没有消费者， 超时判断断开连接
  if (!libtime::Since(stc.tick, ncloseTime)) {
    return;
  }
  stc.business->stop();
  _businessMap.erase(it);
}