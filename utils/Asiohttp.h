#ifndef __ASIO_HTTP_H__
#define __ASIO_HTTP_H__

#include "AsioBase.h"
#include <iostream>

class Asiohttp {
private:
    asio::io_service _ioService;
    const char*      _host;
    const char*      _port;
    unsigned int     _httpStatus;
    std::string      _httpVer;

public:
    Asiohttp(/* args */) {}
    ~Asiohttp() {}

    void initAddress(const char* host, const char* port)
    {
        _host = host;
        _port = port;
    }

    int post(const char* page, const std::string& data, std::string& reponsedata)
    {

        try {
            asio::ip::tcp::socket sock = doNetSocket();
            if (!sock.is_open()) {
                reponsedata = "access net error";
            }
            doNetAccess("POST", page, sock, data);
            return doNetReponse(sock, reponsedata);
        }
        catch (std::exception& e) {
            reponsedata = e.what();
        }
        return -1;
    }

    int get(const char* page, const std::string& data, std::string& reponsedata)
    {
        try {
            asio::ip::tcp::socket sock = doNetSocket();
            if (!sock.is_open()) {
                reponsedata = "access net error";
            }
            doNetAccess("GET", page, sock, data);
            return doNetReponse(sock, reponsedata);
        }
        catch (std::exception& e) {
            reponsedata = e.what();
        }
        return -1;
    }

private:
    // doNetSocket()
    asio::ip::tcp::socket doNetSocket()
    {
        if (_ioService.stopped()) {
            _ioService.reset();
        }
        // 从dns取得域名下的所有ip
        asio::ip::tcp::resolver           resolver(_ioService);
        asio::ip::tcp::resolver::query    query(_host, _port);
        asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

        // 尝试连接到其中的某个ip直到成功
        asio::ip::tcp::socket socket(_ioService);
        asio::connect(socket, endpoint_iterator);
        return socket;
    }

    // POST GET
    int doNetAccess(const char* method, const char* page, asio::ip::tcp::socket& socket, std::string data)
    {
        asio::ip::tcp::resolver resolver(_ioService);
        asio::streambuf         request;
        std::ostream            reqStream(&request);
        reqStream << method << " " << page << " HTTP/1.0\r\n";
        reqStream << "Host: " << _host << ":" << _port << "\r\n";
        reqStream << "Accept: */*\r\n";
        reqStream << "Content-Length: " << data.length() << "\r\n";
        reqStream << "Content-Type: application/x-www-form-urlencoded\r\n";
        reqStream << "Connection: close\r\n\r\n";
        reqStream << data;
        // Send the request.
        asio::write(socket, request);
    }
    // 解析返回数据
    int doNetReponse(asio::ip::tcp::socket& socket, std::string& respData)
    {
        // Read the response status line. The response streambuf will automatically
        // grow to accommodate the entire line. The growth may be limited by passing
        // a maximum size to the streambuf constructor.
        asio::streambuf response;
        asio::read_until(socket, response, "\r\n");

        response.data();
        std::istream response_stream(&response);
        response_stream >> _httpVer;
        response_stream >> _httpStatus;

        std::string status_message;
        std::getline(response_stream, status_message);

        std::cout << status_message.c_str() << "\n";

        if (!response_stream || _httpVer.substr(0, 5) != "HTTP/") {
            respData = "invalid response";
            return -2;
        }
        // 如果服务器返回非200都认为有错,不支持301/302等跳转
        if (_httpStatus != 200) {
            respData = "response returned with status code != 200 ";
            return _httpStatus;
        }

        // 传说中的包头可以读下来了
        std::string              header;
        std::vector<std::string> headers;
        while (std::getline(response_stream, header) && header != "\r")
            headers.push_back(header);

        // 读取所有剩下的数据作为包体
        std::error_code error;
        while (asio::read(socket, response, asio::transfer_at_least(1), error)) {
        }
        //响应有数据
        if (response.size()) {
            std::istream                   response_stream(&response);
            std::istreambuf_iterator<char> eos;
            respData = std::string(std::istreambuf_iterator<char>(response_stream), eos);
        }
        if (error != asio::error::eof) {
            respData = error.message();
            return -3;
        }
        return 0;
    }
};

#endif