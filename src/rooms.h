#pragma once

#include "shared_state.h"
#include <mutex>
#include <shared_mutex>
#include <memory>
#include <unordered_map>
#include <map>

class websocket_session;

/*
struct User {

    std::vector< all_clients>
}
*/

enum class ROOMS_STATUS { OK, 
                          UNKNOWN_ROOM,
                          DUPLICATE_ROOM, 
                          INVALID_ROOM_PASSWORD,
                          INVALID_USER,
                          INVALID_USER_PASSWORD };

std::string status_string(const ROOMS_STATUS& status);

// Probably should be in a database...
class room {
public:
    room();
    room( const std::string& room_name, 
          const std::string& room_password, 
          const std::string& admin,
          const std::string& admin_password );

    room( room& r );
    room& operator=( room& r );
   
    ROOMS_STATUS add_member( const std::string& room_password,
                             const std::string& user_name,
                             const std::string& user_password,
                             websocket_session* session );

    void remove_member( const std::string& user_name );

    void remove_session( const std::string& user_name,
                         websocket_session* session );

    void send( std::string message );

private:

    std::string room_name_;
    std::string room_password_; // I'll do security stuff later

    std::string admin_;
    std::string admin_password_; // Add an admin password

    std::mutex mutex_;

    size_t session_count_;

    std::unordered_map< std::string, std::string > users_;
    std::unordered_map< std::string, std::vector< websocket_session* > > members_;
};

class rooms {
public:
    explicit rooms();

    // Create room then join it
    ROOMS_STATUS create_room( const std::string& room_name,
                              const std::string& room_password, 
                              const std::string& admin,
                              const std::string& admin_password );

    ROOMS_STATUS add_to_room( const std::string& room_name,
                              const std::string& room_password,
                              const std::string& user_name,
                              const std::string& user_password,
                              websocket_session* session );

    ROOMS_STATUS remove_from_room( const std::string& room_name,
                                   const std::string& user_name,
                                   websocket_session* session );

    ROOMS_STATUS send_to_room( const std::string& message,
                               const std::string& room_name);

private:
    std::shared_mutex mutex_;

    std::unordered_map< std::string, room > rooms_;
};
