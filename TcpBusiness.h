#ifndef TCP_BUSINESSS_H
#define TCP_BUSINESSS_H

#include "CommDef.h"
#include "Publisher.h"
#include "TcpConn.h"
#include <string>

namespace ho {
MsgHeader_t newResponse(unsigned short codeId, int len);
std::string doParseRegister(const char* data, int len, std::string& ss);
}  // namespace ho

class TcpBusiness : public TcpConn {
private:
    Publisher_Ptr _publisher;
    bool          _isPublisherWait;
    std::string   _sessionId;

public:
    TcpBusiness(asio::ip::tcp::socket& socket);
    TcpBusiness(asio::io_service& context);
    virtual ~TcpBusiness();
    virtual int  dispatchMessage(char* data, int len);
    virtual void online();
    virtual void offline();

private:
    // 消息头
    int doRspMsg(unsigned short code, const char* data = NULL, int dataLen = 0);
    // 处理心跳
    int doRspHeartbeat();
    // 媒体链路注册响应
    int doRspMediaRegister(char* req, int len);
};

typedef std::shared_ptr<TcpBusiness> TcpBusiness_Ptr;

#endif