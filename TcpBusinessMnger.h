#ifndef TCP_BUSINESS_MNGER_H
#define TCP_BUSINESS_MNGER_H

#include "TcpBusiness.h"
#include "ThMutex.h"
#include <map>
#include <string>

class TcpBusinessMnger {
public:
    typedef struct {
        unsigned long long tick;
        TcpBusiness_Ptr    business;
    } TcpBusiness_t;

public:
    TcpBusinessMnger(/* args */);
    ~TcpBusinessMnger();
    static TcpBusinessMnger* ins();

    bool add(std::string sessionId, TcpBusiness_Ptr ss);
    void remove(std::string sessionId, int nclient, int ncloseTime);

private:
    typedef std::map<std::string, TcpBusiness_t> TyMapTcpBusiness;

    TyMapTcpBusiness _businessMap;
    CommMutex        _mtx;
};

#endif