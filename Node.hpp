#ifndef NODE_HPP
#define NODE_HPP

#include <vector>

#include "Coordinate.hpp"

struct Node
{
    Coordinate coordinate;
    std::vector<Node *> nexts;
    Node *previous;
    char prior_move;
    size_t visit_id = 0;
    int cost_key;
    Node(Coordinate coordinate) : coordinate(coordinate) {}
};

struct NodePtrMinHeapComparator
{
    bool operator()(Node *a, Node *b) { return a->cost_key > b->cost_key; }
};

#endif
