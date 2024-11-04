#ifndef VOXELGRAPH_HPP
#define VOXELGRAPH_HPP

#include "Coordinate.hpp"
#include "BinaryMinHeap.hpp"
#include "Utils.hpp"
#include "Errors.hpp"

class VoxelGraph
{
    // Member Variables
    int x_limit, y_limit, z_limit; // TODO find a way to const correct
    size_t map_area, map_volume;

    Node **node_map;
    BinaryMinHeap *open_set1, *open_set2;

    size_t current_visit_id = 0;

    // Helper Functions
    u_int8_t hex_to_dec(char hex) const { return (hex <= '9') ? hex - '0' : hex - 'W'; }

    bool is_out_of_bounds(Coordinate coordinate) const { return coordinate.x < 0 || coordinate.x >= x_limit ||
                                                                coordinate.y < 0 || coordinate.y >= y_limit ||
                                                                coordinate.z < 0 || coordinate.z >= z_limit; }

    size_t coordinate_to_index(Coordinate coordinate) const { return coordinate.x + x_limit * (coordinate.y + y_limit * coordinate.z); }

public:
    VoxelGraph(std::istream &stream);
    ~VoxelGraph();

    std::string GBeFS(const Coordinate &source, const Coordinate &target);
    std::string RGBeFS(const Coordinate &source, const Coordinate &target);
    std::string BDGBeFS(const Coordinate &source, const Coordinate &target);
    std::string EHBDGBeFS(const Coordinate &source, const Coordinate &target);
};

#endif
