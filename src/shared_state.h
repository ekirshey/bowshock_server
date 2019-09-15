#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>

// Forward declaration
class websocket_session;

// Represents the shared server state
class shared_state
{
    // This mutex synchronizes all access to sessions_
    std::mutex mutex_;

    // Keep a list of all the connected clients
    std::unordered_set<websocket_session*> sessions_;

public:
    explicit shared_state();

    void join  (websocket_session* session);
    void leave (websocket_session* session);
    void send  (std::string message);
};
