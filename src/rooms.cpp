#include "rooms.h"
// Add ALL of the error checking in a bit

room::room()
{

}

room::room( const std::string& name, 
            const std::string& password, 
            const std::string& admin )
    : name_( name )
    , password_( password )
    , admin_( admin )
{

}

room::room( room& r )
{
    std::scoped_lock lock(r.mutex_, mutex_);
    name_ = r.name_;
    admin_ = r.admin_;
    password_ = r.password_;
    members_ = r.members_;
}

room& room::operator=( room& r )
{
    std::scoped_lock lock(r.mutex_, mutex_);
    name_ = r.name_;
    admin_ = r.admin_;
    password_ = r.password_;
    members_ = r.members_;

    return *this;
}

void room::add_member(const std::string& user, websocket_session* session )
{
    std::lock_guard<std::mutex> lock(mutex_);
    members_[user] = session;
}

void room::remove_member(const std::string& user )
{
    std::lock_guard<std::mutex> lock(mutex_);
    members_.erase(user);
}

void room::send( std::string message )
{
    
}

////////////////////////////////////////////////////////

rooms::rooms()
{

}

void rooms::create_room( const std::string& name, 
                         const std::string& password, 
                         const std::string& admin )
{
    std::unique_lock<std::shared_mutex> lock(mutex_);
    rooms_[name] = room(name, password, admin);
}

// Return shared_ptr to room?
void rooms::add_to_room( std::string room, 
                         std::string user, 
                         websocket_session* session )
{
    std::shared_lock<std::shared_mutex> lock(mutex_);
    rooms_[room].add_member(user, session);
}

void rooms::remove_from_room( std::string room, 
                              std::string user, 
                              websocket_session* session )
{
    std::shared_lock<std::shared_mutex> lock(mutex_);
    rooms_[room].remove_member(user);
}

void rooms::send_to_room( std::string message, std::string room )
{
    std::shared_lock<std::shared_mutex> lock(mutex_);
    rooms_[room].send(message);
}