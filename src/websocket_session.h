#pragma once

#include "net.h"
#include "beast.h"
#include "shared_state.h"

#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

// Forward declaration
class shared_state;

/** Represents an active WebSocket connection to the server
*/
class websocket_session : public boost::enable_shared_from_this<websocket_session>
{
    beast::flat_buffer buffer_;
    websocket::stream<beast::tcp_stream> ws_;
    boost::shared_ptr<shared_state> state_;
    std::vector<boost::shared_ptr<std::string const>> queue_;

    void fail(beast::error_code ec, char const* what);
    void on_accept(beast::error_code ec);
    void on_read(beast::error_code ec, std::size_t bytes_transferred);
    void on_write(beast::error_code ec, std::size_t bytes_transferred);

public:
    websocket_session(
        tcp::socket&& socket,
        boost::shared_ptr<shared_state> const& state);

    ~websocket_session();

    void run();

    // Send a message
    void send(const boost::shared_ptr< const std::string >& ss);

private:
    void on_send(const boost::shared_ptr< const std::string >& ss);
};