#pragma once
#include "AsioBase.h"
//
#include "buffer.hpp"
#include "Types.h"
#include <string>

struct bytesArray {
  char* buf;
  size_t length;
};

class TcpConn : public std::enable_shared_from_this<TcpConn> {
 private:
  asio::ip::tcp::socket _socket;
  char _buffer[BUFSIZ];
  libyte::Buffer _recvBuffer;
  libyte::Buffer _sendBuffer;
  std::atomic_bool _writelock;

 public:
  TcpConn(asio::ip::tcp::socket& socket);
  TcpConn(asio::io_service& context);
  virtual ~TcpConn();
  asio::ip::tcp::socket& socket();

  void start();
  void stop();
  bool writeBytes(bytesArray* data, int size);
  // dispatchMessage return vaild data length; if < 0 An error occurred, if = 0
  // need more data
  virtual int dispatchMessage(char* data, size_t len) = 0;
  virtual void online() = 0;
  virtual void offline() = 0;

 private:
  bool processReceiveData();
  void actionRead();
  void actionWrite();
  void handleClose();
};
