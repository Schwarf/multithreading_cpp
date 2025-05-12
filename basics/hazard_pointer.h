//
// Created by andreas on 24.04.23.
//

#ifndef HAZARD_POINTER_H
#define HAZARD_POINTER_H
#include <cstddef>
#include <stdexcept>

constexpr std::size_t max_hazard_pointers = 50;

struct HazardPointer
{
    std::atomic<std::thread::id> id;
    std::atomic<void*> pointer;
};

inline HazardPointer hazard_pointers[max_hazard_pointers];

class HazardPointerOwner
{
    HazardPointer* hazard_pointer;

public:
    HazardPointerOwner(HazardPointerOwner const&) = delete;
    HazardPointerOwner operator=(HazardPointerOwner const&) = delete;

    HazardPointerOwner() : hazard_pointer(nullptr)
    {
        for (auto& hp : hazard_pointers)
        {
            std::thread::id old_id;
            if (hp.id.compare_exchange_strong(
                old_id, std::this_thread::get_id()))
            {
                hazard_pointer = &hp;
                break;
            }
        }
        if (!hazard_pointer)
        {
            throw std::out_of_range("No hazard pointers available!");
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

#endif //HAZARD_POINTER_H

