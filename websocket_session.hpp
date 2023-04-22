#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <iostream>
namespace beast = boost::beast;
namespace websocket = boost::beast::websocket;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

#ifndef CHATROOM_WEBSOCKET_SESSION_HPP
#define CHATROOM_WEBSOCKET_SESSION_HPP

void
fail (beast::error ec, char const* what) {
    std::cerr << what << " : " << ec.message() << "\n";
}

//Echoes back all received WebSocket Messages
class session: public std::enable_shared_from_this<session>
{
    websocket::stream<beast::tcp_stream> ws_;
    beast::flat_buffer buffer_;

public:
    // Take ownership of the socket
    explicit
    session(tcp::socket&& socket)
    : ws_(std::move(socket))
    {}

    // Get on the correct executor
    void run()
    {
        // We need to be executing within a strand to perform async operations
        // on the I/O objects in this session. Although not strictly necessary
        // for single-threaded contexts, this example code is written to be
        // thread-safe by default.
        net::dispath(ws_.get_executor(),
                     beast::bind_front_handler(
                             &session::on_run,
                             shared_from_this()));
    }
};

#endif //CHATROOM_WEBSOCKET_SESSION_HPP
