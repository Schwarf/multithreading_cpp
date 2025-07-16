//
// Created by andreas on 24.04.23.
//

#ifndef HAZARD_POINTER_H
#define HAZARD_POINTER_H
#include <cstddef>
#include <stdexcept>

/// Maximum number of hazard pointers available globally.
/// Each thread may claim one hazard pointer slot at a time.
constexpr std::size_t max_hazard_pointers = 50;

/**
 * @struct HazardPointer
 * @brief A single hazard pointer record.
 *
 * Contains an owner thread identifier and a pointer to the protected object.
 */struct HazardPointer
{
    /// ID of the thread that currently owns this hazard pointer.
    std::atomic<std::thread::id> id;
    /// The actual pointer being protected.
    /// Other threads must check all hazard pointers before reclaiming memory.
    std::atomic<void*> pointer;
};

/// Global array of hazard pointer records. All threads share this pool.
inline HazardPointer hazard_pointers[max_hazard_pointers];

/**
 * @class HazardPointerOwner
 * @brief RAII wrapper to claim and release a hazard pointer slot.
 *
 * Upon construction, searches the global pool for an unused slot
 * and marks it with the current thread's ID. Upon destruction,
 * it clears the pointer and releases the slot back to the pool.
 */
class HazardPointerOwner
{
    HazardPointer* hazard_pointer;

public:
    // Disable copy semantics; only one owner per thread.
    HazardPointerOwner(HazardPointerOwner const&) = delete;
    HazardPointerOwner operator=(HazardPointerOwner const&) = delete;

     /**
     * @brief Construct and claim an unused hazard pointer.
     * @throws std::out_of_range if no slots are available.
     */
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
