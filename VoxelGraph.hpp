#ifndef VOXELGRAPH_HPP
#define VOXELGRAPH_HPP

#include "Coordinate.hpp"
#include "Node.hpp"
#include "BinaryHeap.hpp"
#include "TravelPlan.hpp"
#include "Route.hpp"
#include "Errors.hpp"

static constexpr uint8_t bit_mask[] = {0b1000, 0b0100, 0b0010, 0b0001};
class VoxelGraph
{
    // member variables
    int x_limit, y_limit, z_limit; // TODO find a way to const correct
    size_t map_area, map_volume;

    Node **node_map;
    BinaryHeap<Node *, NodePtrMinHeapComparator> *open_set1, *open_set2;

    size_t current_visit_id = 0;

    // helper functions
    u_int8_t hex_to_dec(char hex) const { return (hex <= '9') ? hex - '0' : hex - 'W'; }

    bool not_in_bounds(Coordinate c) const { return c.x < 0 || c.x >= x_limit || c.y < 0 || c.y >= y_limit || c.z < 0 || c.z >= z_limit; }

    size_t coordinate_to_index(Coordinate c) const { return c.x + x_limit * (c.y + y_limit * c.z); }

public:
    VoxelGraph(std::istream &stream);

    size_t node_count() const;
    size_t find_max_distance() const;
    std::vector<TravelPlan> find_all_travel_plans(size_t minimum_distance = 2) const;

    // path find funcitons
    Route GBeFS(const Coordinate &source, const Coordinate &target);     // greedy best first search
    Route RGBeFS(const Coordinate &source, const Coordinate &target);    // reverse GBeFS
    Route BDGBeFS(const Coordinate &source, const Coordinate &target);   // bidirectional GBeFS
    Route EHBDGBeFS(const Coordinate &source, const Coordinate &target); // evolving-heuristic BDGBeFS

    ~VoxelGraph();
};

#endif
