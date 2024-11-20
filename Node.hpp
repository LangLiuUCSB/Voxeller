#ifndef NODE_HPP
#define NODE_HPP

#include "Coordinate.hpp"
#include "SuperNode.hpp"

#include <vector>

struct Node
{
    const Coordinate coordinate;
    struct Move
    {
        Node *node;
        char direction;
        Move() {};
        Move(Node *node, char direction) : node(node), direction(direction) {};
    };
    std::vector<Move> next_moves;
    std::vector<Move> next_moves_T;

    SuperNode *super_node;

    size_t visit_id = 0;
    size_t visit_id_T = 0;
    Move last_move;
    Move last_move_T;

    int cost_key;
    Node(Coordinate coordinate) : coordinate(coordinate) {}
};

struct NodePtrMinHeapComparator
{
    bool operator()(Node *a, Node *b) { return a->cost_key < b->cost_key; }
};

#endif
