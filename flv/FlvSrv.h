#ifndef TCPFLVSRV_H
#define TCPFLVSRV_H

#include "FlvConn.h"

class FlvServer : public asio::noncopyable {
    asio::ip::tcp::acceptor _acceptor;

public:
    FlvServer(asio::io_context& ioCcontext, int port)
        : _acceptor(ioCcontext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
    {
        doAccept();
    }

private:
    void doAccept()
    {
        _acceptor.async_accept([this](std::error_code ec, asio::ip::tcp::socket socket) {
            if (!ec) {
                std::make_shared<FlvConn>(std::move(socket))->start();
            }
            doAccept();
        });
    }
};

#endif  // !TCPSRVFLV_H
