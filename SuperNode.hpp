#ifndef SUPERNODE_HPP
#define SUPERNODE_HPP

#include "Node.hpp"

struct SuperNode
{
    std::vector<struct Node *> nodes;

    struct Move
    {
        SuperNode *super_node;
        Node *exit_node, *entry_node;
        char direction;
        Move() {};
        Move(SuperNode *super_node, Node *exit_node, Node *entry_node, char direction)
            : super_node(super_node), exit_node(exit_node), entry_node(entry_node), direction(direction) {};
    };
    std::vector<Move> next_moves;
    std::vector<Move> next_moves_T;

    size_t visit_id = 0;
    size_t visit_id_T = 0;
    Move last_move;

    int cost_key;
};

struct SuperNodePtrMinHeapComparator
{
    bool operator()(SuperNode *a, SuperNode *b) { return a->cost_key < b->cost_key; }
};

#endif
