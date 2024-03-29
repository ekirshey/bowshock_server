#include "rooms.h"
#include "websocket_session.h"

#include <iostream>

room::room()
{

}

room::room( const std::string& room_name,
            const std::string& room_password,
            const std::string& admin,
            const std::string& admin_password )
    : room_name_( room_name )
    , room_password_( room_password )
    , admin_( admin )
    , admin_password_( admin_password )
    , session_count_( 0 )
{

}

// Manual implementation of the copy constructors due to the mutex
room::room( room& r )
{
    std::scoped_lock lock(r.mutex_, mutex_);
    room_name_ = r.room_name_;
    admin_ = r.admin_;
    room_password_ = r.room_password_;
    session_count_ = r.session_count_;
    members_ = r.members_;
}

room& room::operator=( room& r )
{
    std::scoped_lock lock(r.mutex_, mutex_);
    room_name_ = r.room_name_;
    admin_ = r.admin_;
    room_password_ = r.room_password_;
    session_count_ = r.session_count_;
    members_ = r.members_;

    return *this;
}

SERVER_STATUS room::add_member(const std::string& room_password,
                               const std::string& user_name,
                               websocket_session* session )
{
    std::lock_guard<std::mutex> lock(mutex_);

    if ( room_password == room_password_ )
    {
        if (members_.find(user_name) != members_.end())
        {
            members_[user_name].emplace_back( session );
        }
        else
        {
            members_[user_name] = { session };
        }

        session_count_++;
        
        return SERVER_STATUS::OK;
    }

    return SERVER_STATUS::INVALID_ROOM_PASSWORD;
}

void room::remove_member( const std::string& user_name)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if ( members_.find(user_name) != members_.end() )
    {
        session_count_ -= members_[user_name].size();
        members_.erase(user_name);
    }
}

void room::remove_session( const std::string& user_name,
                           websocket_session* session )
{
    if (members_.find(user_name) != members_.end())
    {
        auto& v = members_[user_name];
        size_t temp = members_[user_name].size();
        v.erase( std::remove(v.begin(), v.end(), session), v.end() );
        session_count_ -= (temp - members_[user_name].size());

        // If member has no more sessions, remove it
        if (members_[user_name].size() == 0) {
            members_.erase(user_name);
        }
    }
}

void room::send( std::string message )
{
    // Put the message in a shared pointer so we can re-use it for each client
    auto const ss = std::make_shared<std::string const>(std::move(message));

    // Make a local list of all the weak pointers representing
    // the sessions, so we can do the actual sending without
    // holding the mutex:
    std::vector<std::weak_ptr<websocket_session>> v;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        std::cout << "Clients to send to " << session_count_ << std::endl;
        v.reserve(session_count_);
        for (auto&[k, r] : members_)
        {
            for (auto& s : r)
            {
                v.emplace_back(s->weak_from_this());
            }
        }
    }

    // For each session in our local list, try to acquire a strong
    // pointer. If successful, then send the message on that session.
    for (auto const& wp : v)
        if (auto sp = wp.lock())
            sp->send(ss);
}

std::vector<std::string> room::get_members()
{
    std::lock_guard<std::mutex> lock(mutex_);

    std::vector<std::string> members;
    for (auto&[k, r] : members_)
    {
        members.push_back(k);
    }

    return members;
}

////////////////////////////////////////////////////////

rooms::rooms()
{

}

SERVER_STATUS rooms::create_room( const std::string& room_name,
                                 const std::string& room_password, 
                                 const std::string& admin,
                                 const std::string& admin_password )
{
    std::unique_lock<std::shared_mutex> lock(mutex_);
    if (rooms_.find(room_name) == rooms_.end())
    {
        rooms_[room_name] = room(room_name, room_password, admin, admin_password);
        std::cout << " Created room: " << room_name << " for user: " << admin << std::endl;
        return SERVER_STATUS::OK;
    }
    // How to convey to user that the room failed?
    // Should I have an error code instead of bool
    return SERVER_STATUS::DUPLICATE_ROOM;
}

// Return shared_ptr to room?
SERVER_STATUS rooms::add_to_room( const std::string& room_name,
                                 const std::string& room_password,
                                 const std::string& user_name,
                                 websocket_session* session )
{
    std::shared_lock<std::shared_mutex> lock(mutex_);

    if (rooms_.find(room_name) == rooms_.end())
        return SERVER_STATUS::UNKNOWN_ROOM;

    auto res = rooms_[room_name].add_member(room_password, 
                                            user_name,
                                            session );

    // Inform all other members of the room
    if (res == SERVER_STATUS::OK) {
        auto members = rooms_[room_name].get_members();
        auto req = member_update_msg(room_name, members);
        rooms_[room_name].send(req);
    }

    return res;
}

SERVER_STATUS rooms::remove_from_room( const std::string& room_name,
                                      const std::string& user_name,
                                      websocket_session* session )
{
    // Unique lock in case of cleaning up room
    std::unique_lock<std::shared_mutex> lock(mutex_);

    if (rooms_.find(room_name) == rooms_.end())
        return SERVER_STATUS::UNKNOWN_ROOM;

    rooms_[room_name].remove_session(user_name, session);

    if (rooms_[room_name].session_count() <= 0)
    {
        rooms_.erase(room_name);
    }
    else {
        // Inform all other members of the room
        auto members = rooms_[room_name].get_members();
        auto req = member_update_msg(room_name, members);
        rooms_[room_name].send(req);
    }

    return SERVER_STATUS::OK;
}

SERVER_STATUS rooms::send_to_room( const std::string& message,
                                  const std::string& room_name)
{
    std::shared_lock<std::shared_mutex> lock(mutex_);

    if (rooms_.find(room_name) == rooms_.end())
        return SERVER_STATUS::UNKNOWN_ROOM;

    rooms_[room_name].send(message);

    return SERVER_STATUS::OK;
}

SERVER_STATUS rooms::get_members(const std::string& room_name,
                                std::vector<std::string>& members)
{
    std::shared_lock<std::shared_mutex> lock(mutex_);

    if (rooms_.find(room_name) == rooms_.end())
        return SERVER_STATUS::UNKNOWN_ROOM;

    members = rooms_[room_name].get_members();

    return SERVER_STATUS::OK;
}