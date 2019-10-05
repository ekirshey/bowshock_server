#include "websocket_session.h"
#include <nlohmann/json.hpp>
#include <iostream>

websocket_session::websocket_session( 
                                    tcp::socket&& socket,
                                    std::shared_ptr<rooms> const& rooms )
    : ws_(std::move(socket))
    , rooms_(rooms)
    , current_user_("")
    , current_room_("")
{
}

websocket_session::
~websocket_session()
{
    std::cout << "Disconnect session" << std::endl;
    // Leave room
    if (current_room_.size() != 0)
    {
        rooms_->remove_from_room( current_room_, 
                                  current_user_,
                                  this );
    }
}

void
websocket_session::
fail(beast::error_code ec, char const* what)
{
    // Don't report these
    if( ec == net::error::operation_aborted ||
        ec == websocket::error::closed)
        return;

    std::cerr << what << ": " << ec.message() << "\n";
}

void
websocket_session::
on_accept(beast::error_code ec)
{
    // Handle the error, if any
    if(ec)
        return fail(ec, "accept");

    std::cout << " New connection accepted" << std::endl;

    // Read a message
    ws_.async_read(
        buffer_,
        beast::bind_front_handler(
            &websocket_session::on_read,
            shared_from_this()));
}

void
websocket_session::
on_read(beast::error_code ec, std::size_t)
{
    // Handle the error, if any
    if (ec)
        return fail(ec, "read");

    std::string data(beast::buffers_to_string(buffer_.data()));

    // Make the business logic class!
    // Convert message to json
    auto body = nlohmann::json::parse(data);
    // MessageTypes
    std::cout << " Parsing received message " << std::endl;
    if (body.at("message_type") == MESSAGE_TYPES::CREATE_ROOM)
    {
        nlohmann::json resp;
        std::string room_name = body.at("room_name");
        std::string room_password = body.at("room_password");
        std::string user_name = body.at("user_name");
        std::string user_password = body.at("user_password");
        auto res = rooms_->create_room(room_name,
            room_password,
            user_name,
            user_password);

        if (res == SERVER_STATUS::OK)
        {
            if (current_room_.size() != 0)
            {
                // Remove from current room
                rooms_->remove_from_room(current_room_, user_name, this);
            }
            // Created new room so add user to it
            auto res = rooms_->add_to_room(room_name,
                room_password,
                user_name,
                user_password,
                this);
            if (res == SERVER_STATUS::OK)
            {
                current_user_ = user_name;
                current_room_ = room_name;
            }
        }

        resp = create_room_resp(user_name, room_name, res);

        auto const ss = std::make_shared<std::string const>(std::move(resp.dump()));
        send(ss);
    }
    else if (body.at("message_type") == MESSAGE_TYPES::JOIN_ROOM)
    {
        nlohmann::json resp;

        std::string room_name = body.at("room_name");
        std::string room_password = body.at("room_password");
        std::string user_name = body.at("user_name");
        std::string user_password = body.at("user_password");
        auto res = rooms_->add_to_room(room_name,
            room_password,
            user_name,
            user_password,
            this);

        std::cout << "Add to room result: " << (int)res << std::endl;
        std::vector<std::string> members;
        if (res == SERVER_STATUS::OK)
        {
            if (current_room_.size() != 0)
            {
                // Remove from current room
                rooms_->remove_from_room(current_room_, user_name, this);
            }

            std::cout << " Added to room: " << room_name << " for user: " << user_name << std::endl;
            current_user_ = user_name;
            current_room_ = room_name;

            
            rooms_->get_members(current_room_, members);
        }

        resp = join_room_resp(user_name, room_name, members, res);

        auto const ss = std::make_shared<std::string const>(std::move(resp.dump()));
        send(ss);
    }
    else if (body.at("message_type") == MESSAGE_TYPES::LEAVE_ROOM)
    {
        nlohmann::json resp;
        SERVER_STATUS res = SERVER_STATUS::INVALID_USER;
        if (current_room_.size() != 0)
        {
            // How do I handle the owner leaving?
            res = rooms_->remove_from_room( current_room_,
                                            current_user_,
                                            this);
            current_user_ = "";
            current_room_ = "";
        }

        resp = leave_room_resp(res);
        auto const ss = std::make_shared<std::string const>(std::move(resp.dump()));
        send(ss);
    }
    else if ( current_room_.size() != 0 )
    {
        std::cout << "Routing normal message to room members" << std::endl;
        rooms_->send_to_room(data, current_room_);
    }

    // Clear the buffer
    buffer_.consume(buffer_.size());

    // Read another message
    ws_.async_read(
        buffer_,
        beast::bind_front_handler(
            &websocket_session::on_read,
            shared_from_this()));
}

void websocket_session::run()
{
    // Set suggested timeout settings for the websocket
    ws_.set_option(
        websocket::stream_base::timeout::suggested(
            beast::role_type::server));

    // Set a decorator to change the Server of the handshake
    ws_.set_option(websocket::stream_base::decorator(
        [](websocket::response_type& res)
    {
        res.set(http::field::server,
            std::string(BOOST_BEAST_VERSION_STRING) +
            " bowshock-editor-server");
    }));

    // Accept the websocket handshake
    ws_.async_accept(
        beast::bind_front_handler(
            &websocket_session::on_accept,
            shared_from_this()));
}

void websocket_session::send(std::shared_ptr<std::string const> const& ss)
{
    // Post our work to the strand, this ensures
    // that the members of `this` will not be
    // accessed concurrently.

    net::post(
        ws_.get_executor(),
        beast::bind_front_handler(
            &websocket_session::on_send,
            shared_from_this(),
            ss));
}

void websocket_session::on_send(std::shared_ptr<std::string const> const& ss)
{
    // Always add to queue
    queue_.push_back(ss);

    // Are we already writing?
    if(queue_.size() > 1)
        return;

    // We are not currently writing, so send this immediately
    ws_.async_write(
        net::buffer(*queue_.front()),
        beast::bind_front_handler(
            &websocket_session::on_write,
            shared_from_this()));
}

void websocket_session::on_write(beast::error_code ec, std::size_t)
{
    // Handle the error, if any
    if(ec)
        return fail(ec, "write");

    // Remove the string from the queue
    queue_.erase(queue_.begin());

    // Send the next message if any
    if(! queue_.empty())
        ws_.async_write(
            net::buffer(*queue_.front()),
            beast::bind_front_handler(
                &websocket_session::on_write,
                shared_from_this()));
}
