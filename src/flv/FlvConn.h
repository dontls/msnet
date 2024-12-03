#ifndef FLVCONNECTION_H
#define FLVCONNECTION_H

#include "AsioBase.h"

class FlvConn : public std::enable_shared_from_this<FlvConn> {
 private:
  asio::ip::tcp::socket _socket;
  char _buffer[BUFSIZ];
  unsigned long long _startTick;
  std::string _sessionId;
  bool bWaitKey_;

 public:
  FlvConn(asio::ip::tcp::socket socket);
  void start();
  void Write(char* data, size_t n, bool bmeta, std::string& aacspec);

 private:
  void doRead();
  void doWrite(const char* data, int len);
  void doClose();
  bool flvHTTPUrlParse(const char* request, int len);
};

typedef std::shared_ptr<FlvConn> FlvConn_Ptr;

#endif
