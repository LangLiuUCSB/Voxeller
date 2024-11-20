#ifndef VOXELGRAPH_HPP
#define VOXELGRAPH_HPP

#include "Coordinate.hpp"
#include "Node.hpp"
#include "SuperNode.hpp"
#include "TravelPlan.hpp"
#include "Route.hpp"
#include "Errors.hpp"

#include <iostream>
#include <queue>
#include <functional>

static constexpr uint8_t bit_mask[] = {0b1000, 0b0100, 0b0010, 0b0001};
class VoxelGraph
{
    // member variables
    int x_limit, y_limit, z_limit; // TODO find a way to const correct
    size_t map_area, map_volume;

    Node **node_map;
    std::vector<SuperNode *> super_node_list;

    size_t current_visit_id = 0;

    // helper functions
    u_int8_t hex_to_dec(char hex) const { return (hex <= '9') ? hex - '0' : hex - 'W'; }

    bool not_in_bounds(Coordinate c) const { return c.x < 0 || c.x >= x_limit || c.y < 0 || c.y >= y_limit || c.z < 0 || c.z >= z_limit; }
    size_t coordinate_to_index(Coordinate c) const { return c.x + x_limit * (c.y + y_limit * c.z); }
    bool visited(Node *node) const { return node->visit_id == current_visit_id; }
    bool visited(SuperNode *super_node) const { return super_node->visit_id == current_visit_id; }

    void DFS1(Node *current_node, std::stack<Node *> &st) const;
    void DFS2(Node *current_node_inverse, SuperNode *super_node) const;

    Route GBFS(Node *source_node, Node *target_node) const;
    Route RGBFS(Node *source_node, Node *target_node) const;

public:
    VoxelGraph() {}
    VoxelGraph(std::istream &stream);

    void condense_graph();

    // path find funcitons
    Route greedy_best_first_search(const Coordinate &source, const Coordinate &target);
    Route super_greedy_best_first_search(const Coordinate &source, const Coordinate &target);

    Route reverse_greedy_best_first_search(const Coordinate &source, const Coordinate &target);
    Route reverse_super_greedy_best_first_search(const Coordinate &source, const Coordinate &target);

    Route bidirectional_best_first_search(const Coordinate &source, const Coordinate &target);

    ~VoxelGraph();

    // benchmarking tools
    size_t node_count() const
    {
        size_t count = 0;
        for (size_t i = map_area; i < map_volume; ++i)
            if (node_map[i] != nullptr && i == coordinate_to_index(node_map[i]->coordinate))
                ++count;
        return count;
    }
    std::vector<Coordinate> find_all_valid_position() const
    {
        std::vector<Coordinate> positions;
        for (size_t i = map_area; i < map_volume; ++i)
            if (node_map[i] != nullptr && i == coordinate_to_index(node_map[i]->coordinate))
                positions.emplace_back(node_map[i]->coordinate);
        return positions;
    }
    size_t find_max_distance() const
    {
        size_t max_distance = 0;
        for (size_t i = map_area; i < map_volume; ++i)
            if (node_map[i] != nullptr)
                for (size_t j = map_area; j < map_volume; ++j)
                    if (node_map[j] != nullptr && node_map[i]->coordinate.manhattan_distance(node_map[j]->coordinate) > max_distance)
                        max_distance = node_map[i]->coordinate.manhattan_distance(node_map[j]->coordinate);
        return max_distance;
    }
    std::vector<TravelPlan> find_all_travel_plans(size_t minimum_distance) const
    {
        std::vector<TravelPlan> travel_plans;
        for (size_t i = map_area; i < map_volume; ++i)
            if (node_map[i] != nullptr)
                for (size_t j = map_area; j < map_volume; ++j)
                    if (node_map[j] != nullptr && node_map[i]->coordinate.manhattan_distance(node_map[j]->coordinate) >= minimum_distance)
                        travel_plans.emplace_back(node_map[i]->coordinate, node_map[j]->coordinate);
        return travel_plans;
    }
};

#endif
