#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

enum class MESSAGE_TYPES {
    CREATE_ROOM = 0,
    JOIN_ROOM,
    LEAVE_ROOM,
    MEMBERS_UPDATE
};

enum class SERVER_STATUS {
    OK = 200,
    BAD = 460,
    UNKNOWN_ROOM = 461,
    DUPLICATE_ROOM = 462,
    INVALID_ROOM_PASSWORD = 463,
    INVALID_USER = 464,
    INVALID_USER_PASSWORD = 465
};

std::string status_string(const SERVER_STATUS& status);

nlohmann::json create_room_resp(const std::string& user_name,
                                const std::string& room_name,
                                SERVER_STATUS result);

nlohmann::json join_room_resp(const std::string& user_name,
                              const std::string& room_name,
                              const std::vector<std::string>& members,
                              SERVER_STATUS result);

nlohmann::json leave_room_resp(SERVER_STATUS result);

std::string member_update_msg(const std::string& room,
                              const std::vector<std::string>& members);