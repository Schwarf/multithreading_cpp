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
            // Try to install this thread's ID into an empty slot.
            // If the id of hp is empty it has not yet been claimed
            if (std::thread::id no_owner; hp.id.compare_exchange_strong(no_owner, std::this_thread::get_id()))
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

    /**
     * @brief Access the underlying atomic pointer for protection.
     * @return Reference to the thread‑local atomic<void*> hazard pointer.
     */
    std::atomic<void*>& get_pointer() const
    {
        return hazard_pointer->pointer;
    }

    /**
     * @brief Release the hazard pointer and free the slot.
     *
     * Clears the pointer and resets the owner ID, making the slot
     * available for other threads.
     */
    ~HazardPointerOwner()
    {
        hazard_pointer->pointer.store(nullptr);
        hazard_pointer->id.store(std::thread::id());
    }
};


/**
 * @brief Get the calling thread's hazard pointer.
 *
 * Internally creates a thread‑local HazardPointerOwner, so each thread
 * only ever claims one slot.  Returns a reference to the protected pointer.
 *
 * @return Reference to a thread‑local hazard pointer atomic.
 */
inline std::atomic<void*>& get_hazard_pointer()
{
    thread_local HazardPointerOwner hazard;
    return hazard.get_pointer();
}

#endif //HAZARD_POINTER_H
