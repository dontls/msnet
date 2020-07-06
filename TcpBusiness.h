#ifndef TCP_BUSINESSS_H
#define TCP_BUSINESSS_H

#include "CommDef.h"
#include <string>
namespace ho {
MsgHeader_t NewResponse(unsigned short codeId, int len);
std::string DoRegisterMediaLink(const char* data, int len, std::string& ss);
}  // namespace ho
#endif