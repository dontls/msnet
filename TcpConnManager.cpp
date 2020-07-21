#include "TcpConnManager.h"
#include "Time.h"

TcpConnManager::TcpConnManager(/* args */) {}

TcpConnManager::~TcpConnManager() {}

TcpConnManager* TcpConnManager::ins()
{
    static TcpConnManager ins;
    return &ins;
}

bool TcpConnManager::addTcpConn(std::string sessionId, TcpConn_Ptr ptr)
{
    CommRWLock lock(&_mtx);
    auto       it = _tcpConnMap.find(sessionId);
    if (it != _tcpConnMap.end()) {
        return false;
    }
    STcpConn_t tConn;
    tConn.tick = NowTickCount();
    tConn.tcpConn = ptr;
    _tcpConnMap.insert(TyMapTcpConn::value_type(sessionId, tConn));
    return true;
}

void TcpConnManager::removeTcpConn(std::string sessionId, int nclient, int nCloseTime)
{
    CommRWLock lock(&_mtx);
    auto       it = _tcpConnMap.find(sessionId);
    if (it == _tcpConnMap.end()) {
        return;
    }
    STcpConn_t stc = it->second;
    // 存在消费者， 更新时间戳
    if (nclient > 1) {
        stc.tick = NowTickCount();
        return;
    }
    // 没有消费者， 超时判断断开连接
    if (TimeoutSec(stc.tick, nCloseTime)) {
        stc.tcpConn->setManualClose();
        _tcpConnMap.erase(it);
    }
}