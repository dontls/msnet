#ifndef TCPCON_MANAGER_H
#define TCPCON_MANAGER_H

#include "TcpConn.h"
#include "ThMutex.h"
#include <map>
#include <string>

class TcpConnManager {
public:
    typedef struct {
        unsigned long long tick;
        TcpConn_Ptr        tcpConn;
    } STcpConn_t;

public:
    TcpConnManager(/* args */);
    ~TcpConnManager();
    static TcpConnManager* ins();

    bool addTcpConn(std::string sessionId, TcpConn_Ptr ss);
    void removeTcpConn(std::string sessionId, int nclient, int nCloseTime);

private:
    typedef std::map<std::string, STcpConn_t> TyMapTcpConn;

    TyMapTcpConn _tcpConnMap;
    CommMutex    _mtx;
};

#endif