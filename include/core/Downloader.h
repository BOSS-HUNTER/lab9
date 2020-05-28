#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <string>

class Downloader
{
public:
    using Resolver = boost::asio::ip::tcp::resolver;
    using Socket = boost::asio::ip::tcp::socket;
    using IoContext = boost::asio::io_context;
    using SslContext = boost::asio::ssl::context;
    using SslStream = boost::asio::ssl::stream<Socket>;

    Downloader(const Downloader &) = delete;
    Downloader(Downloader &&) = delete;

    Downloader &operator=(const Downloader &) = delete;
    Downloader &operator=(Downloader &&) = delete;

    // connect
    Downloader(std::string host, std::string port);

    [[nodiscard]] std::string getHost() const
    {
        return host_;
    }

    [[nodiscard]] std::string getPort() const
    {
        return port_;
    }

    std::string download(const std::string &url, int httpVersion = 10);

private:
    std::string host_;
    std::string port_;

    IoContext ioContext;
    SslContext sslContext{SslContext::sslv23_client};       // Context for SSL/TLS client

    Resolver resolver{ioContext};
    SslStream stream{ioContext, sslContext};

    Resolver::results_type resolverResults{};
};