#pragma once

#include "shared_state.h"
#include <mutex>
#include <shared_mutex>
#include <memory>
#include <unordered_map>

// Probably should be in a database...
class room {
public:
    room();
    room( const std::string& name, 
          const std::string& password, 
          const std::string&admin );

    room( room& r );
    room& operator=( room& r );
   
    void add_member( const std::string&, websocket_session* session );
    void remove_member( const std::string& user );

    void send( std::string message );
private:
    std::string name_;
    std::string password_; // I'll do security stuff later
    std::string admin_;

    std::mutex mutex_;

    std::unordered_map<std::string, websocket_session*> members_;
};

class rooms {
public:
    explicit rooms();

    void create_room( const std::string& name, 
                      const std::string& password, 
                      const std::string& admin );

    void add_to_room( std::string room, 
                      std::string user, 
                      websocket_session* session );

    void remove_from_room( std::string room, 
                           std::string user, 
                           websocket_session* session );

    void send_to_room(std::string message, std::string room);

private:
    std::shared_mutex mutex_;

    std::unordered_map< std::string, room > rooms_;
};
