#include "TcpConn.h"
#include "log.hpp"

const int _maxBufferSize = 1024 * 500;

TcpConn::TcpConn(asio::ip::tcp::socket& socket)
    : _socket(std::move(socket)),
      _recvBuffer(_maxBufferSize),
      _sendBuffer(_maxBufferSize) {}

TcpConn::TcpConn(asio::io_service& context)
    : _socket(context),
      _recvBuffer(_maxBufferSize),
      _sendBuffer(_maxBufferSize) {}

TcpConn::~TcpConn() {}
void TcpConn::start() {
  _writelock = false;
  asio::ip::tcp::no_delay noDelay(true);
  _socket.set_option(noDelay);
  online();
  actionRead();
}

asio::ip::tcp::socket& TcpConn::socket() { return _socket; }

void TcpConn::stop() {
  asio::error_code ignored_ec;
  _socket.shutdown(asio::ip::tcp::socket::shutdown_both, ignored_ec);
}

bool TcpConn::writeBytes(bytesArray* data, int size) {
  for (int i = 0; i < size; i++) {
    if (_sendBuffer.Write(data[i].buf, data[i].length) != data[i].length) {
      return false;
    }
  }
  actionWrite();
  return true;
}

bool TcpConn::processReceiveData() {
  for (;;) {
    int vaildLength = dispatchMessage(_recvBuffer.Bytes(), _recvBuffer.Len());
    if (vaildLength < 0) {
      return false;
    }
    if (vaildLength == 0) {
      break;
    }
    _recvBuffer.Remove(vaildLength);
  }
  return true;
}

void TcpConn::actionRead() {
  if (!processReceiveData()) {
    handleClose();
    return;
  }
  auto bufptr = asio::buffer(_buffer, BUFSIZ);
  auto self = shared_from_this();
  _socket.async_read_some(
      bufptr, [this, self](std::error_code ec, std::size_t bytes_transferred) {
        if (!ec) {
          _recvBuffer.Write(_buffer, bytes_transferred);
          actionRead();
        } else {
          LogError(1, "async_read_some error %d\n", ec.value());
          handleClose();
        }
      });
}

void TcpConn::actionWrite() {
  bool expectedVal = false;
  if (!_writelock.compare_exchange_strong(expectedVal, true)) {
    LogError(1, "async_write busy\n");
    return;
  }
  int len = _sendBuffer.Len();
  if (len <= 0) {
    _writelock = false;
    return;
  }
  auto self = shared_from_this();
  auto bufptr = asio::buffer(_sendBuffer.Bytes(), len);
  asio::async_write(
      _socket, bufptr,
      [this, self](std::error_code ec, std::size_t bytes_transferred) {
        _writelock = false;
        if (!ec) {
          _sendBuffer.Remove(bytes_transferred);
          actionWrite();
        } else {
          LogError(1, "async_write error %d\n", ec.value());
          handleClose();
        }
      });
}

void TcpConn::handleClose() {
  offline();
  asio::error_code ignored_ec;
  _socket.shutdown(asio::ip::tcp::socket::shutdown_both, ignored_ec);
}