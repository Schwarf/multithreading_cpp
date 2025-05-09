//
// Created by andreas on 02.05.25.
//

#ifndef RECLAMATION_H
#define RECLAMATION_H
#include "hazard_pointer.h"

template <typename T>
void do_delete(void* pointer)
{
    delete static_cast<T*>(pointer);
}

struct data_to_reclaim
{
    void* data;
    std::function<void(void*)> deleter;
    data_to_reclaim* next;

    template <typename T>
    explicit data_to_reclaim(T* pointer):
        data(pointer),
        deleter(&do_delete<T>),
        next(nullptr)
    {
    }

    ~data_to_reclaim()
    {
        deleter(data);
    }
};

inline std::atomic<data_to_reclaim*> nodes_to_reclaim;


inline void add_to_reclaim_list(data_to_reclaim* node)
{
    node->next = nodes_to_reclaim.load();
    while (!nodes_to_reclaim.compare_exchange_weak(node->next, node));
}

template <typename T>
void reclaim_later(T* data)
{
    add_to_reclaim_list(new data_to_reclaim(data));
}

inline void delete_nodes_with_no_hazards()
{
    data_to_reclaim* current = nodes_to_reclaim.exchange(nullptr);
    while (current)
    {
        data_to_reclaim* const next = current->next;
        if (!outstanding_hazard_pointers_for(current->data))
        {
            delete current;
        }
        else
        {
            add_to_reclaim_list(current);
        }
        current = next;
    }
}

#endif //RECLAMATION_H
