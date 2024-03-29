#include "server_schema.h"

std::string status_string(const SERVER_STATUS& status)
{
    switch (status)
    {
    case SERVER_STATUS::OK:
        return "OK";
    case SERVER_STATUS::UNKNOWN_ROOM:
        return "UNKNOWN_ROOM";
    case SERVER_STATUS::DUPLICATE_ROOM:
        return "DUPLICATE_ROOM";
    case SERVER_STATUS::INVALID_ROOM_PASSWORD:
        return "INVALID_ROOM_PASSWORD";
    case SERVER_STATUS::INVALID_USER:
        return "INVALID_USER";
    case SERVER_STATUS::DUPLICATE_USER:
        return "DUPLICATE_USER";
    case SERVER_STATUS::INVALID_USER_PASSWORD:
        return "INVALID_USER_PASSWORD";
    case SERVER_STATUS::NO_LOGIN:
        return "NO_LOGIN";
    default:
        return "UNKNOWN";
    }
}

nlohmann::json create_room_resp(const std::string& user_name,
                                const std::string& room_name,
                                SERVER_STATUS result)
{
    return {
        {"type" , MESSAGE_TYPES::CREATE_ROOM},
        {"user_name" , user_name},
        {"room_name" , room_name},
        {"result" , result},
        {"result_str", status_string(result)}
    };
}

nlohmann::json join_room_resp(const std::string& user_name,
                              const std::string& room_name,
                              const std::vector<std::string>& members,
                              SERVER_STATUS result)
{
    return {
        {"type" , MESSAGE_TYPES::JOIN_ROOM},
        {"user_name" , user_name},
        {"room_name" , room_name},
        {"members", members},
        {"result" , result},
        {"result_str", status_string(result)}
    };
}

nlohmann::json leave_room_resp(SERVER_STATUS result)
{
    return {
        {"type", MESSAGE_TYPES::LEAVE_ROOM},
        { "result" , result },
        { "result_str", status_string(result) }
    };
}

nlohmann::json add_user_resp(const std::string& user_name, SERVER_STATUS result)
{
    return {
        {"type" , MESSAGE_TYPES::ADD_NEW_USER},
        {"user_name" , user_name},
        {"result" , result},
        {"result_str", status_string(result)}
    };
}

nlohmann::json auth_user_resp(const std::string& user_name, SERVER_STATUS result)
{
    return {
        {"type" , MESSAGE_TYPES::AUTHENTICATE_USER},
        {"user_name" , user_name},
        {"result" , result},
        {"result_str", status_string(result)}
    };
}

std::string member_update_msg(const std::string& room,
                              const std::vector<std::string>& members)
{
    nlohmann::json req = {
        {"type" , MESSAGE_TYPES::MEMBERS_UPDATE},
        {"room" , room},
        {"members", members}
    };

    return req.dump();
}