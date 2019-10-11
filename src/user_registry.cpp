#include "user_registry.h"

user_registry::user_registry() {

}

SERVER_STATUS user_registry::add_new_user(const std::string& username,
                                          const std::string& password)
{
    std::unique_lock<std::shared_mutex> lock(mutex_);

    if (users_.find(username) != users_.end())
    {
        return SERVER_STATUS::DUPLICATE_USER;
    }
    
    users_[username] = password;

    return SERVER_STATUS::OK;
}

SERVER_STATUS user_registry::authenticate_user(const std::string& username,
                                               const std::string& password)
{
    std::shared_lock<std::shared_mutex> lock(mutex_);

    if (users_.find(username) == users_.end())
    {
        return SERVER_STATUS::INVALID_USER;
    }

    if (users_[username] != password) {
        return SERVER_STATUS::INVALID_USER_PASSWORD;
    }

    return SERVER_STATUS::OK;
}