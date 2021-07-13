#ifndef TCP_CONN_H
#define TCP_CONN_H
#pragma once
#include "AsioBase.h"
//
#include "ByteBuffer.h"
#include "CommDef.h"
#include <string>

typedef struct bytesArray {
    char* buf;
    int   length;
};

class TcpConn : public std::enable_shared_from_this<TcpConn> {
private:
    asio::ip::tcp::socket _socket;
    char                  _buffer[BUFSIZ];
    bytes::Buffer         _recvBuffer;
    bytes::Buffer         _sendBuffer;
    std::atomic_bool      _writelock;

public:
    TcpConn(asio::ip::tcp::socket& socket);
    TcpConn(asio::io_service& context);
    virtual ~TcpConn();
    asio::ip::tcp::socket& socket();

    void start();
    void stop();
    bool writeBytes(bytesArray* data, int size);
    // dispatchMessage return vaild data length; if < 0 An error occurred, if = 0 need more data
    virtual int  dispatchMessage(char* data, int len) = 0;
    virtual void online() = 0;
    virtual void offline() = 0;

private:
    bool processReceiveData();
    void actionRead();
    void actionWrite();
    void handleClose();
};

#endif  // !TcpConn_H
