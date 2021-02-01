#include "TcpBusiness.h"
#include "nlohmann/json.hpp"
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