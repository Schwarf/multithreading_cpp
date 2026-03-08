//
// Created by andreas on 08.03.26.
//

#ifndef SHARED_MEMORY_CHAT_COMMON_H
#define SHARED_MEMORY_CHAT_COMMON_H

#include <atomic>
#include <cstddef>

inline constexpr const char* SHM_NAME = "/my_shared_chat";
inline constexpr std::size_t MESSAGE_SIZE = 256;

struct SharedChat {
    std::atomic<bool> message_ready{false};
    std::atomic<bool> shutdown{false};
    char message[MESSAGE_SIZE]{};
};

#endif //SHARED_MEMORY_CHAT_COMMON_H