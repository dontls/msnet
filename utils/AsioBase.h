#pragma once

#include "3rd/asio-1.12.2/asio.hpp"
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
