#pragma once

#define ASIO_STANDALONE
#define ASIO_HAS_STD_CHRONO

#include <asio.hpp>
#include <memory>
#include <vector>

struct noncopyable {
 protected:
  noncopyable() {}
  virtual ~noncopyable() {}

 private:
  noncopyable(const noncopyable&) = delete;
  noncopyable& operator=(const noncopyable&) = delete;
  noncopyable(noncopyable&&) = delete;
  noncopyable& operator=(noncopyable&&) = delete;
};
