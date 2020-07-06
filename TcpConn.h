#ifndef TCP_CONN_H
#define TCP_CONN_H
#pragma once
#include "AsioBase.h"
//
#include "ByteBuffer.h"
#include "CommDef.h"
#include "Publisher.h"
#include <string>

class TcpConn : public std::enable_shared_from_this<TcpConn> {
private:
    asio::ip::tcp::socket  _socket;
    std::array<char, 8192> _buffer;
    std::string            _sessionId;
    bytes::Buffer          _recvBuffer;
    bytes::Buffer          _sendBuffer;
    Publisher_Ptr          _publisher;
    bool                   _isPublisherWait;
    bool                   _isAutoClose;

public:
    TcpConn(asio::ip::tcp::socket socket);
    TcpConn(asio::io_service& context);
    ~TcpConn();

    void                   start();
    bool                   dispatchMessage(char* data, int len);
    asio::ip::tcp::socket& socket();

    void autoClose();

private:
    void doRead();
    void doSocketWrite(char* data, int len);
    void doSocketBufferWrite(int len);
    // 消息头
    int doRspMsgHeader(ho::MsgHeader_t msg);
    // 处理心跳
    int doRspHeartbeat();
    // 媒体链路注册响应
    int doRspMediaRegister(char* req, int len);
};
typedef std::shared_ptr<TcpConn> TcpConn_Ptr;

#endif  // !TcpConn_H
