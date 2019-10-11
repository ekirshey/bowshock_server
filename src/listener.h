#pragma once

#include "beast.h"
#include "net.h"
#include "rooms.h"
#include "user_registry.h"
#include <memory>
#include <string>

// Forward declaration
class shared_state;

// Accepts incoming connections and launches the sessions
class listener : public std::enable_shared_from_this<listener>
{
    net::io_context& ioc_;
    tcp::acceptor acceptor_;
    std::shared_ptr<user_registry> user_registry_;
    std::shared_ptr<rooms> rooms_;
    
    void fail(beast::error_code ec, char const* what);
    void on_accept(beast::error_code ec, tcp::socket socket);

public:
    listener(
        net::io_context& ioc,
        tcp::endpoint endpoint,
        std::shared_ptr<user_registry> const& user_registry,
        std::shared_ptr<rooms> const& rooms);

    // Start accepting incoming connections
    void run();
};