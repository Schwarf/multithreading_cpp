//
// Created by andreas on 08.03.26.
//
#include "common.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>

namespace
{
    [[noreturn]] void throw_system_error(const std::string& msg)
    {
        throw std::runtime_error(msg + ": " + std::strerror(errno));
    }
}

int main()
{
    constexpr std::size_t shared_memory_size = sizeof(SharedChat);

    int file_descriptor = shm_open(SHM_NAME, O_RDWR, 0666);
    if (file_descriptor == -1)
    {
        throw_system_error("shm_open failed (start writer first)");
    }

    void* mapped_memory = mmap(nullptr, shared_memory_size, PROT_READ | PROT_WRITE, MAP_SHARED, file_descriptor, 0);
    if (mapped_memory == MAP_FAILED)
    {
        close(file_descriptor);
        throw_system_error("mmap failed");
    }

    auto* chat = static_cast<SharedChat*>(mapped_memory);

    std::cout << "Reader started. Waiting for messages...\n";

    while (true)
    {
        if (chat->message_ready.load(std::memory_order_acquire))
        {
            std::cout << "Received: " << chat->message << '\n';
            chat->message_ready.store(false, std::memory_order_release);
        }

        if (chat->shutdown.load(std::memory_order_acquire))
        {
            std::cout << "Writer requested shutdown.\n";
            break;
        }

        usleep(10'000);
    }

    if (munmap(mapped_memory, shared_memory_size) == -1)
    {
        std::cerr << "munmap failed: " << std::strerror(errno) << '\n';
    }

    if (close(file_descriptor) == -1)
    {
        std::cerr << "close failed: " << std::strerror(errno) << '\n';
    }

    return 0;
}
