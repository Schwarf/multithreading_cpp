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

    void addToRetiredNodes(RetiredNode* retiredNode)
    {
        retiredNode->next = RetiredNodes.load();
        while (!RetiredNodes.compare_exchange_strong(retiredNode->next, retiredNode));
    }

public:
    bool isInUse(Node* node)
    {
        for (auto& hp : hazard_pointers)
        {
            if (hp.pointer.load() == node) return true;
        }
        return false;
    }

    void addNode(Node* node)
    {
        addToRetiredNodes(new RetiredNode(node));
    }

    void deleteUnusedNodes()
    {
        RetiredNode* current = RetiredNodes.exchange(nullptr);
        while (current)
        {
            RetiredNode* const next = current->next;
            if (!isInUse(current->node)) delete current;
            else addToRetiredNodes(current);
            current = next;
        }
    }
};



#endif //RECLAMATION_H
