#include "TcpBusiness.h"
#include "TcpBusinessMnger.h"
#include "nlohmann/json.hpp"
#include <memory>
namespace ho {
ho::MsgHeader_t newResponse(unsigned short codeId, int len)
{
    ho::MsgHeader_t pMsg;
    pMsg.ucFlag = ho::MsgFlag;
    pMsg.ucVersion = ho::MsgVersion;
    pMsg.usCodeID = codeId;
    pMsg.unPayloadLen = len;
    return pMsg;
}

std::string doParseRegister(const char* data, int len, std::string& ss)
{
    nlohmann::json req = nlohmann::json::parse(std::string(data, len));
    ss = req["ss"].get<std::string>();
    nlohmann::json js = { { "ss", ss }, { "err", 0 } };
    return js.dump();
}
}  // namespace ho

TcpBusiness::TcpBusiness(asio::ip::tcp::socket& socket) : TcpConn(socket) {}

TcpBusiness::TcpBusiness(asio::io_service& context) : TcpConn(context) {}

TcpBusiness::~TcpBusiness() {}

void TcpBusiness::online()
{
    _isPublisherWait = true;
    _publisher = std::make_shared<Publisher>();
}

void TcpBusiness::offline()
{
    LOG("%s close\n", _sessionId.c_str());
    TcpBusinessMnger::ins()->remove(_sessionId, 0, 0);
}

int TcpBusiness::dispatchMessage(char* data, int len)
{
    if (len < ho::MsgHeaderLen) {
        return 0;
    }
    ho::MsgHeader_t* pMsgHeader = ( ho::MsgHeader_t* )data;
    if (pMsgHeader->ucFlag != ho::MsgFlag || pMsgHeader->ucVersion != ho::MsgVersion) {
        return -1;
    }
    if (len < (ho::MsgHeaderLen + pMsgHeader->unPayloadLen)) {
        return 0;
    }
    char* palyloadData = data + ho::MsgHeaderLen;
    switch (pMsgHeader->usCodeID) {
    case 0x1002:  //
        doRspMediaRegister(palyloadData, pMsgHeader->unPayloadLen);
        break;
    case 0x0001:
        doRspHeartbeat();
        break;
    case 0x0011: {
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
            _publisher->recvAudioRaw(palyloadData + 12, pMsgHeader->unPayloadLen - 12, pMeHeader->ullFrameTimeStamp);
            break;
        default:
            break;
        }
    } break;
    default:
        break;
    }
    return ho::MsgHeaderLen + pMsgHeader->unPayloadLen;
}

// 消息头
int TcpBusiness::doRspMsg(unsigned short code, const char* data, int dataLen)
{
    if (NULL == data) {
        dataLen = 0;
    }
    ho::MsgHeader_t header = ho::newResponse(code, dataLen);
    writeBytes(( char* )&header, ho::MsgHeaderLen);
    if (data) {
        writeBytes(( char* )data, dataLen);
    }
    return ho::MsgHeaderLen + dataLen;
}

// 处理心跳
int TcpBusiness::doRspHeartbeat()
{
    return doRspMsg(0x0001, NULL, 0);
}

// 媒体链路注册响应
int TcpBusiness::doRspMediaRegister(char* req, int len)
{
    LOG("%s\n", req);
    std::string payloadStr = ho::doParseRegister(req, len, _sessionId);
    if (payloadStr.empty()) {
        return 0;
    }
    if (_publisher->initSession(_sessionId)) {
        _isPublisherWait = false;
    }
    // 这里把设备添加到管理列表
    TcpBusiness_Ptr ptr = std::dynamic_pointer_cast<TcpBusiness>(shared_from_this());
    TcpBusinessMnger::ins()->add(_sessionId, ptr);
    return doRspMsg(0x4002, payloadStr.c_str(), payloadStr.length());
}