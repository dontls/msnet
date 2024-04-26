#include "FlvConn.h"
#include "FlvWriter.h"
#include "log.hpp"
#include "string.hpp"

const char HTTPFlv_Header[] = {0x46, 0x4c, 0x56, 0x01, 0x01, 0x00, 0x00,
                               0x00, 0x09, 0x00, 0x00, 0x00, 0x00};

const std::string HTTPFlv_Rsp =
    "HTTP/1.1 200 OK\r\n"
    "Cache-Control: no-cache\r\n"
    "Content-Type: flash\r\n"
    "Connection: keep-alive\r\n"
    "Expires: -1\r\n"
    "Access-Control-Allow-Origin: *\r\n"
    "Pragma: no-cache\r\n"
    "\r\n";

FlvConn::FlvConn(asio::ip::tcp::socket socket)
    : _socket(std::move(socket)), _isWaitMeta(true) {}
void FlvConn::start() {
  asio::ip::tcp::no_delay noDelay(true);
  _socket.set_option(noDelay);
  doRead();
}

void FlvConn::rawWriteFlvPacket(const char* data, uint32_t len,
                                const char* aacSpec, uint32_t aacSpecLen,
                                bool isMeta) {
  // 等待sps/pps 并且直发一次
  if (_isWaitMeta && isMeta) {
    _isWaitMeta = false;
  }
  if (_isWaitMeta) {
    return;
  }
  if (isMeta) {
    doWrite(aacSpec, aacSpecLen);
  }
  doWrite(data, len);
}

// http://localhost:10600/live/session1222222222222222222222.flv
bool FlvConn::flvHTTPUrlParse(const char* request, int len) {
  std::istringstream istr(request);
  std::string firstLine;
  std::getline(istr, firstLine);
  LogDebug("%s\n", firstLine.c_str());
  std::vector<std::string> v = libstr::Split(firstLine, " ");
  if (v.size() != 3) {
    return false;
  }
  std::string method = v[0];
  if (method.compare("GET") != 0) {
    return false;
  }
  std::string url = v[1];
  std::string::size_type pos = std::string::npos;
  if (url.find(".flv") == pos) {
    return false;
  }

  pos = url.find_last_of("/");
  const char* sstr = url.c_str() + pos + 1;
  std::string ss(sstr, strlen(sstr) - 4);
  _sessionId = ss;
  return true;
}

void FlvConn::doRead() {
  auto self(shared_from_this());
  _socket.async_read_some(
      asio::buffer(_buffer, BUFSIZ),
      [this, self](std::error_code ec, std::size_t bytes_transferred) {
        if (!ec && flvHTTPUrlParse(_buffer, bytes_transferred)) {
          LogDebug("\n%s\n", _buffer);
          doWrite(HTTPFlv_Rsp.c_str(),
                  HTTPFlv_Rsp.length());  // 1、http请求响应 Content_Length:
                                          // 不设置client可以一直接收数据
          doWrite(HTTPFlv_Header, sizeof(HTTPFlv_Header));  // 2、发送flv头
          FlvWriter_Ptr flvWriter =
              FlvWriterManager::ins()->flvWriter(_sessionId);  // 3、添加到列表
          if (flvWriter != nullptr) {
            flvWriter->addFlvWriterConn(shared_from_this());
          }
        } else {
          LogDebug("async_read_some error %d\n", ec.value());
          doClose();
        }
      });
}

void FlvConn::doWrite(const char* data, int len) {
  auto self(shared_from_this());
  asio::async_write(_socket, asio::buffer(data, len),
                    [this, self](std::error_code ec, std::size_t) {
                      if (!ec) {
                      } else {
                        LogDebug("async_write error %d\n", ec.value());
                        doClose();
                      }
                      // if (ec != asio::error::operation_aborted) {
                      //     // connection_manager_.stop(shared_from_this());
                      // }
                    });
  // asio::write(_socket, asio::buffer(data, len));
}

void FlvConn::doClose() {
  asio::error_code ignored_ec;
  _socket.shutdown(asio::ip::tcp::socket::shutdown_both, ignored_ec);
  FlvWriter_Ptr flvWriter = FlvWriterManager::ins()->flvWriter(_sessionId);
  if (flvWriter) {
    flvWriter->delFlvWriterConn(shared_from_this());
  }
}