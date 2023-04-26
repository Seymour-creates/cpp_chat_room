#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = boost::beast::websocket;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

#ifndef CHATROOM_WEBSOCKET_SESSION_HPP
#define CHATROOM_WEBSOCKET_SESSION_HPP



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

    void
    fail (beast::error_code ec, char const* what) {
        std::cerr << what << " : " << ec.message() << "\n";
    }

    // Get on the correct executor
    void run()
    {
        // We need to be executing within a strand to perform async operations
        // on the I/O objects in this session. Although not strictly necessary
        // for single-threaded contexts, this example code is written to be
        // thread-safe by default.
        net::dispatch(ws_.get_executor(),
                     beast::bind_front_handler(
                             &session::on_run,
                             shared_from_this()));
    }

    // Start the asynchronous operation
    void on_run()
    {
        // Set suggested timeout settings for the websocket
        ws_.set_option(websocket::stream_base::timeout::suggested(
                beast::role_type::server));
        // Set decorator to change the server of the handshake
        ws_.set_option(websocket::stream_base::decorator(
                [](websocket::response_type& res) {
                    res.set(http::field::server,
                            std::string(BOOST_BEAST_VERSION_STRING) + " websocket-server--async");
                }));
        //Accept the websocket handshake
        ws_.async_accept(
                beast::bind_front_handler(
                        &session::on_accept,
                        shared_from_this()));
    }

    void on_accept(beast::error_code ec)
    {
        if (ec) return fail(ec, "accept");

        // read a message
        do_read();
    }

    void do_read()
    {
        // Read a message into our buffer
        ws_.async_read(buffer_,
                       beast::bind_front_handler(
                               &session::on_read,
                               shared_from_this()));
    }

    void on_read( beast::error_code ec, std::size_t bytes_transferred )
    {
        boost::ignore_unused(bytes_transferred);

        // This indicates that the session was closed
        if (ec == websocket::error::closed) return;

        if ( ec ) return fail( ec, "read" );

        // Echo the message
        ws_.text(ws_.got_text());
        ws_.async_write( buffer_.data(), beast::bind_front_handler( &session::on_write, shared_from_this() ));
    }

    void on_write( beast::error_code ec, std::size_t bytes_transferred )
    {
        boost::ignore_unused(bytes_transferred);

        if (ec) return fail(ec, "write");

        // Clear the buffer
        buffer_.consume(buffer_.size());

        // Do another read
        do_read();
    }
};

#endif //CHATROOM_WEBSOCKET_SESSION_HPP
