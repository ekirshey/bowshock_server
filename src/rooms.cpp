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
    users_ = r.users_;
    members_ = r.members_;
}

room& room::operator=( room& r )
{
    std::scoped_lock lock(r.mutex_, mutex_);
    room_name_ = r.room_name_;
    admin_ = r.admin_;
    room_password_ = r.room_password_;
    session_count_ = r.session_count_;
    users_ = r.users_;
    members_ = r.members_;

    return *this;
}

/*
If the user isn't in the set add it
Otherwise check for the password and add new session
*/
ROOMS_STATUS room::add_member( const std::string& room_password,
                               const std::string& user_name,
                               const std::string& user_password,
                               websocket_session* session )
{
    std::lock_guard<std::mutex> lock(mutex_);

    if ( room_password == room_password_ )
    {     
        if (users_.find(user_name) != users_.end())
        {
            if (users_[user_name] != user_password)
            {
                return ROOMS_STATUS::INVALID_USER_PASSWORD;
            }
            members_[user_name].emplace_back( session );
        }
        else
        {
            // Create user
            users_[user_name] = user_password;
            members_[user_name] = { session };
        }
        
        session_count_++;
        return ROOMS_STATUS::OK;
    }

    return ROOMS_STATUS::INVALID_ROOM_PASSWORD;
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
        int temp = members_[user_name].size();
        v.erase( std::remove(v.begin(), v.end(), session), v.end() );
        session_count_ -= (temp - members_[user_name].size());
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

////////////////////////////////////////////////////////

rooms::rooms()
{

}

ROOMS_STATUS rooms::create_room( const std::string& room_name,
                                 const std::string& room_password, 
                                 const std::string& admin,
                                 const std::string& admin_password )
{
    std::unique_lock<std::shared_mutex> lock(mutex_);
    if (rooms_.find(room_name) == rooms_.end())
    {
        rooms_[room_name] = room(room_name, room_password, admin, admin_password);
        std::cout << " Created room: " << room_name << " for user: " << admin << std::endl;
        return ROOMS_STATUS::OK;
    }
    // How to convey to user that the room failed?
    // Should I have an error code instead of bool
    return ROOMS_STATUS::DUPLICATE_ROOM;
}

// Return shared_ptr to room?
ROOMS_STATUS rooms::add_to_room( const std::string& room_name,
                                 const std::string& room_password,
                                 const std::string& user_name,
                                 const std::string& user_password,
                                 websocket_session* session )
{
    std::shared_lock<std::shared_mutex> lock(mutex_);

    if (rooms_.find(room_name) == rooms_.end())
        return ROOMS_STATUS::UNKNOWN_ROOM;

    return rooms_[room_name].add_member(room_password, 
                                        user_name,
                                        user_password,
                                        session );
}

ROOMS_STATUS rooms::remove_from_room( const std::string& room_name,
                                      const std::string& user_name,
                                      websocket_session* session )
{
    std::shared_lock<std::shared_mutex> lock(mutex_);

    if (rooms_.find(room_name) == rooms_.end())
        return ROOMS_STATUS::UNKNOWN_ROOM;
    
    rooms_[room_name].remove_member(user_name);

    return ROOMS_STATUS::OK;
}

ROOMS_STATUS rooms::send_to_room( const std::string& message,
                                  const std::string& room_name)
{
    std::shared_lock<std::shared_mutex> lock(mutex_);

    if (rooms_.find(room_name) == rooms_.end())
        return ROOMS_STATUS::UNKNOWN_ROOM;

    rooms_[room_name].send(message);

    return ROOMS_STATUS::OK;
}