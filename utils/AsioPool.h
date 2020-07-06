#pragma once
#include "AsioBase.h"

// @see https://github.com/huyuguang/asio_benchmark
/// A pool of io_service objects.
class IoServicePool : noncopyable {
public:
    /// Construct the io_service pool.
    explicit IoServicePool(std::size_t poolSize) : _nextIoService(0)
    {
        if (poolSize == 0)
            throw std::runtime_error("io_service_pool size is 0");

        // Give all the io_services work to do so that their run() functions will not
        // exit until they are explicitly stopped.
        for (std::size_t i = 0; i < poolSize; ++i) {
            IoService_Ptr ioService(new asio::io_service);
            Work_Ptr       work(new asio::io_service::work(*ioService));
            _ioServices.push_back(ioService);
            _work.push_back(work);
        }
    }

    /// Run all io_service objects in the pool.
    void run()
    {
        // Create a pool of threads to run all of the io_services.
        std::vector<std::shared_ptr<std::thread>> threads;
        for (std::size_t i = 0; i < _ioServices.size(); ++i) {
            std::shared_ptr<std::thread> thread(new std::thread([i, this]() { _ioServices[i]->run(); }));
            threads.push_back(thread);
        }

        // Wait for all threads in the pool to exit.
        for (std::size_t i = 0; i < threads.size(); ++i)
            threads[i]->join();
    }

    /// Stop all io_service objects in the pool.
    void stop()
    {
        // Explicitly stop all io_services.
        for (std::size_t i = 0; i < _work.size(); ++i)
            _work[i].reset();
    }

    /// Get an io_service to use.
    asio::io_service& ioService()
    {
        // Use a round-robin scheme to choose the next io_service to use.
        asio::io_service& io_service = *_ioServices[_nextIoService];
        ++_nextIoService;
        if (_nextIoService == _ioServices.size())
            _nextIoService = 0;
        return io_service;
    }

    asio::io_service& ioService(size_t index)
    {
        index = index % _ioServices.size();
        return *_ioServices[index];
    }

private:
    typedef std::shared_ptr<asio::io_service>       IoService_Ptr;
    typedef std::shared_ptr<asio::io_service::work> Work_Ptr;

    /// The pool of io_services.
    std::vector<IoService_Ptr> _ioServices;

    /// The work that keeps the io_services running.
    std::vector<Work_Ptr> _work;

    /// The next io_service to use for a connection.
    std::size_t _nextIoService;
};