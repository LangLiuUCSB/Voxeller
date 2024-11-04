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
    bool operator<(const Node &other) const { return cost_key < other.cost_key; }
};

#endif
