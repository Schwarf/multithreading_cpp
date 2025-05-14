//
// Created by andreas on 02.05.25.
//

#ifndef RECLAMATION_H
#define RECLAMATION_H

template <typename Node, typename Deleter = std::default_delete<Node>>
class RetireList
{
    struct RetiredNode
    {
        Node* node;
        RetiredNode* next;

        explicit RetiredNode(Node* p) : node(p), next(nullptr)
        {
        }

        ~RetiredNode()
        {
            Deleter{}(node);
        }
    };

    std::atomic<RetiredNode*> RetiredNodes;

    void add_to_retired_nodes(RetiredNode* retiredNode)
    {
        retiredNode->next = RetiredNodes.load();
        while (!RetiredNodes.compare_exchange_strong(retiredNode->next, retiredNode));
    }

public:
    bool is_in_use(Node* node)
    {
        for (auto& hp : hazard_pointers)
        {
            if (hp.pointer.load() == node)
                return true;
        }
        return false;
    }

    void add_node(Node* node)
    {
        add_to_retired_nodes(new RetiredNode(node));
    }

    void delete_unused_nodes()
    {
        RetiredNode* current = RetiredNodes.exchange(nullptr);
        while (current)
        {
            RetiredNode* const next = current->next;
            if (!is_in_use(current->node)) delete current;
            else add_to_retired_nodes(current);
            current = next;
        }
    }
};


#endif //RECLAMATION_H
