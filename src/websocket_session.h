#pragma once

#include "net.h"
#include "beast.h"
#include "shared_state.h"
#include "user_registry.h"
#include "rooms.h"

#include <cstdlib>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <optional>

// Forward declaration
class shared_state;

/** Represents an active WebSocket connection to the server
*/
class websocket_session : public std::enable_shared_from_this<websocket_session>
{
    beast::flat_buffer buffer_;
    websocket::stream<beast::tcp_stream> ws_;
    std::shared_ptr<user_registry> user_registry_;
    std::shared_ptr<rooms> rooms_;

    // How do i know if the websocket has joined a room?
    std::string current_user_;
    std::string current_room_;
    
    std::vector<std::shared_ptr<std::string const>> queue_;

    void fail(beast::error_code ec, char const* what);
    void on_accept(beast::error_code ec);
    void on_read(beast::error_code ec, std::size_t bytes_transferred);
    void on_write(beast::error_code ec, std::size_t bytes_transferred);

public:
    websocket_session(
        tcp::socket&& socket,
        std::shared_ptr<user_registry> const& user_registry,
        std::shared_ptr<rooms> const& rooms);

    ~websocket_session();

    void run();

    // Send a message
    void send(const std::shared_ptr< const std::string >& ss);

private:
    void on_send(const std::shared_ptr< const std::string >& ss);
};