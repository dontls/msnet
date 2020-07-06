#ifndef TCPCON_MANAGER_H
#define TCPCON_MANAGER_H

#include "TcpConn.h"
#include <map>
#include <mutex>
#include <string>
#include <unordered_map>

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

    bool addTcpConn(std::string devID, TcpConn_Ptr ss);
    void removeTcpConn(std::string devID, int nclient, int nCloseTime);

private:
    typedef std::map<std::string, STcpConn_t> TyMapTcpConn;

    TyMapTcpConn _tcpConnMap;
    std::mutex   _mtx;
};

#endif