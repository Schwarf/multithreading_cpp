//
// Created by andreas on 24.04.23.
//

#ifndef HAZARD_POINTER_H
#define HAZARD_POINTER_H
#include <atomic>
#include <thread>
#include <stdexcept>

unsigned int constexpr max_hazard_pointers = 100;

struct HazardPointer
{
    std::atomic<std::thread::id> id{std::thread::id()};
    std::atomic<void*> pointer{nullptr};
};

inline HazardPointer hazard_pointers[max_hazard_pointers] = {};

class HazardPointerOwner
{
    HazardPointer* hazard_pointer;

public:
    HazardPointerOwner(HazardPointerOwner const&) = delete;
    HazardPointerOwner operator=(HazardPointerOwner const&) = delete;

    HazardPointerOwner(): hazard_pointer(nullptr)
    {
        for (auto& hp : hazard_pointers)
        {
            std::thread::id old_id;
            if (hp.id.compare_exchange_strong(old_id, std::this_thread::get_id()))
            {
                hazard_pointer = &hp;
                break;
            }
        }
        if (!hazard_pointer)
        {
            throw std::runtime_error("No hazard pointers available");
        }
    }

    std::atomic<void*>& get_pointer()
    {
        return hazard_pointer->pointer;
    }

    ~HazardPointerOwner()
    {
        hazard_pointer->pointer.store(nullptr);
        hazard_pointer->id.store(std::thread::id());
    }
};

inline std::atomic<void*>& get_hazard_pointer()
{
    thread_local static HazardPointerOwner hazard;
    return hazard.get_pointer();
}

inline bool outstanding_hazard_pointers_for(void* pointer)
{
    for (auto& hazard_pointer : hazard_pointers)
    {
        if (hazard_pointer.pointer.load() == pointer)
        {
            return true;
        }
    }
    return false;
}
#endif //HAZARD_POINTER_H
