#include "TcpConnManager.h"
#include "Time.h"

TcpConnManager::TcpConnManager(/* args */) {}

TcpConnManager::~TcpConnManager() {}

TcpConnManager* TcpConnManager::ins()
{
    static TcpConnManager ins;
    return &ins;
}

bool TcpConnManager::addTcpConn(std::string devID, TcpConn_Ptr ptr)
{
    std::unique_lock<std::mutex> lock(_mtx);
    STcpConn_t tConn;
    tConn.tick = NowTickCount();
    tConn.tcpConn = ptr;
    _tcpConnMap.insert(TyMapTcpConn::value_type(devID, tConn));
    return true;
}

void TcpConnManager::removeTcpConn(std::string devID, int nclient, int nCloseTime)
{
    // std::unique_lock<std::mutex> lock(_mtx);
    // auto       it = _tcpConnMap.find(devID);
    // if (it != _tcpConnMap.end()) {
    //     STcpConn_t*  sl = it->second;
    //     if (nclient <= 1) {
    //         // 没有消费者， 超时判断断开连接
    //         if (TimeoutSec(sl.tick, nCloseTime)) {
    //             sl->tcpConn->autoClose();
    //             _tcpConnMap.erase(it);
    //         }
    //     } else {
    //         // 存在消费者， 更新时间戳
    //         sl->tcpConn = NowTickCount();
    //     }
    // }
}