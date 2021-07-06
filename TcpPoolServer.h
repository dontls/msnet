#ifndef TCP_POOLSERVER_H
#define TCP_POOLSERVER_H

#include "AsioPool.h"
#include "TcpBusiness.h"

class TcpPoolServer : public noncopyable {
    IoServicePool           _ioSrvPool;  // 这里必须定义在_acceptor前面
    asio::ip::tcp::acceptor _acceptor;

public:
    TcpPoolServer(const asio::ip::tcp::endpoint& endpoint, size_t threadCnt)
        : _ioSrvPool(threadCnt), _acceptor(_ioSrvPool.ioService())
    {
        _acceptor.open(endpoint.protocol());
        _acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(1));
        _acceptor.bind(endpoint);
        _acceptor.listen();
    }

    void run()
    {
        doAccept();
        _ioSrvPool.run();
    }

private:
    void doAccept()
    {
        TcpBusiness_Ptr newConn(new TcpBusiness(_ioSrvPool.ioService()));
        auto&       socket = newConn->socket();
        _acceptor.async_accept(socket, [this, newConn = std::move(newConn)](const asio::error_code& err) {
            if (!err) {
                newConn->start();
                doAccept();
            }
        });
    }
};

#endif