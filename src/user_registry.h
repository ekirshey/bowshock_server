#pragma once
#include "server_schema.h"
#include <shared_mutex>
#include <unordered_map>

class user_registry {
public:
    user_registry();

    SERVER_STATUS add_new_user(const std::string& username,
                               const std::string& password);

    SERVER_STATUS authenticate_user(const std::string& username,
                                    const std::string& password);
private:
    std::shared_mutex mutex_;

    std::unordered_map< std::string, std::string > users_;
};
