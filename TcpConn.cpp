#include "TcpConn.h"
#include "Log.h"
#include "TcpBusiness.h"
#include <iostream>
#include "TcpConnManager.h"

TcpConn::TcpConn(asio::ip::tcp::socket socket) : _socket(std::move(socket)), _publisher(std::make_shared<Publisher>()) {}
TcpConn::TcpConn(asio::io_service& context) : _socket(context), _publisher(std::make_shared<Publisher>()) {}
TcpConn::~TcpConn() {}
void TcpConn::start()
{
    _isPublisherWait = true;
    _isAutoClose = false;
    asio::ip::tcp::no_delay noDelay(true);
    _socket.set_option(noDelay);
    doRead();
}

asio::ip::tcp::socket& TcpConn::socket()
{
    return _socket;
}

bool TcpConn::dispatchMessage(char* data, int len)
{
    _recvBuffer.Write(data, len);
    for (;;) {
        int recvLen = _recvBuffer.Len();
        if (recvLen < 8) {
            break;
        }
        char*            recvData = _recvBuffer.Bytes();
        ho::MsgHeader_t* pMsgHeader = ( ho::MsgHeader_t* )recvData;
        if (pMsgHeader->ucFlag != ho::MsgFlag || pMsgHeader->ucVersion != ho::MsgVersion) {
            return false;
        }
        if (recvLen < (ho::MsgHeaderLen + pMsgHeader->unPayloadLen)) {
            break;
        }
        char* palyloadData = recvData + ho::MsgHeaderLen;
        switch (pMsgHeader->usCodeID) {
        case 0x1002:  //
            doSocketBufferWrite(doRspMediaRegister(palyloadData, pMsgHeader->unPayloadLen));
            break;
        case 0x0001:
            // doWrite(recvData, ho::MsgHeaderLen);
            doSocketBufferWrite(doRspHeartbeat());
            break;
        case 0x0011:  //
        {
            if (_isPublisherWait) {
                break;
            }
            ho::MediaHeader_t* pMeHeader = ( ho::MediaHeader_t* )(palyloadData);
            // LOG("channel: %d type:%d timestamp:%lld\n", pMeHeader->usFrameChannel, pMeHeader->usFrameType,
            //        pMeHeader->ullFrameTimeStamp);
            // +12 跳过媒体数据消息头
            switch (pMeHeader->usFrameType) {
            case ho::KFrameType_Video_I:
            case ho::KFrameType_Video_P:
                _publisher->recvVideoRaw(palyloadData + 12, pMsgHeader->unPayloadLen - 12, pMeHeader->usFrameType,
                                             pMeHeader->ullFrameTimeStamp);
                break;
            case ho::KFrameType_Audio:
                _publisher->recvAudioRaw(palyloadData + 12, pMsgHeader->unPayloadLen - 12,
                                             pMeHeader->ullFrameTimeStamp);
                break;
            default:
                break;
            }
        } break;
        default:
            break;
        }
        _recvBuffer.Remove(ho::MsgHeaderLen + pMsgHeader->unPayloadLen);
        // printf("recv buffer Len %d Cap %d\n", _recvBuffer.Len(), _recvBuffer.Cap());
    }
    return true;
}

void TcpConn::autoClose()
{
    _isAutoClose = true;
}

void TcpConn::doRead()
{
    auto self(shared_from_this());
    if (_isAutoClose) {
        LOG("%s Auto Close\n", _sessionId.c_str());
        _socket.close();
        return;
    }
    _socket.async_read_some(asio::buffer(_buffer), [this, self](std::error_code ec, std::size_t bytes_transferred) {
        if (!ec && dispatchMessage(_buffer.data(), bytes_transferred)) {
            doRead();
        } else {
            // TcpConnManager::ins()->removeTcpConn(_sessionId, 0, 0);
            _socket.close();
            LOG("async_read_some error %d\n", ec.value());
        }
    });
}

void TcpConn::doSocketWrite(char* data, int len)
{
    asio::write(_socket, asio::buffer(data, len));
}

void TcpConn::doSocketBufferWrite(int len)
{
    if (len > 0) {
        asio::write(_socket, asio::buffer(_sendBuffer.Bytes(), len));
        _sendBuffer.Remove(len);
    }
}

// 消息头
int TcpConn::doRspMsgHeader(ho::MsgHeader_t msg)
{
    _sendBuffer.Write(( char* )&msg, ho::MsgHeaderLen);
    return ho::MsgHeaderLen;
}

// 处理心跳
int TcpConn::doRspHeartbeat()
{
    return doRspMsgHeader(ho::NewResponse(0x0001, 0));
}

// 媒体链路注册响应
int TcpConn::doRspMediaRegister(char* req, int len)
{
    LOG("%s\n", req);
    std::string payloadStr = ho::DoRegisterMediaLink(req, len, _sessionId);
    if (payloadStr.empty()) {
        return 0;
    }
    if(_publisher->initSession(_sessionId)) {
        _isPublisherWait = false;
    }
    // 这里把设备添加到管理列表
    // TcpConnManager::ins()->addTcpConn(_sessionId, shared_from_this());
    doRspMsgHeader(ho::NewResponse(0x4002, payloadStr.length()));
    _sendBuffer.WriteString(payloadStr);
    return ho::MsgHeaderLen + payloadStr.length();
}