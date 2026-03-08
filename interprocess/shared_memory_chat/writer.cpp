//
// Created by andreas on 08.03.26.
//
#include "common.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

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

    int file_descriptor = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (file_descriptor == -1)
    {
        throw_system_error("shm_open failed");
    }

    if (ftruncate(file_descriptor, static_cast<off_t>(shared_memory_size)) == -1)
    {
        close(file_descriptor);
        shm_unlink(SHM_NAME);
        throw_system_error("ftruncate failed");
    }

    void* mapped_memory = mmap(nullptr, shared_memory_size, PROT_READ | PROT_WRITE, MAP_SHARED, file_descriptor, 0);
    if (mapped_memory == MAP_FAILED)
    {
        close(file_descriptor);
        shm_unlink(SHM_NAME);
        throw_system_error("mmap failed");
    }

    auto* chat = static_cast<SharedChat*>(mapped_memory);

    chat->message_ready.store(false);
    chat->shutdown.store(false);
    std::memset(chat->message, 0, MESSAGE_SIZE);

    std::cout << "Writer started.\n";
    std::cout << "Type messages. Type /exit to quit.\n";

    std::string input;
    while (true)
    {
        std::cout << "> ";
        if (!std::getline(std::cin, input))
        {
            break;
        }

        if (input == "/exit")
        {
            chat->shutdown.store(true, std::memory_order_release);
            break;
        }

        while (chat->message_ready.load(std::memory_order_acquire))
        {
            usleep(10'000);
        }

        std::strncpy(chat->message, input.c_str(), MESSAGE_SIZE - 1);
        chat->message[MESSAGE_SIZE - 1] = '\0';
        chat->message_ready.store(true, std::memory_order_release);
    }

    if (munmap(mapped_memory, shared_memory_size) == -1)
    {
        std::cerr << "munmap failed: " << std::strerror(errno) << '\n';
    }

    if (close(file_descriptor) == -1)
    {
        std::cerr << "close failed: " << std::strerror(errno) << '\n';
    }

    if (shm_unlink(SHM_NAME) == -1)
    {
        std::cerr << "shm_unlink failed: " << std::strerror(errno) << '\n';
    }

    return 0;
}
