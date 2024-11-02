#ifndef VOXELGRAPH_HPP
#define VOXELGRAPH_HPP

#include "Coordinate.hpp"
#include "Path.hpp"
#include "BinaryMinHeap.hpp"
#include "Utils.hpp"
#include "Errors.hpp"

class VoxelGraph
{
    // Member Variables
    int x_limit, y_limit, z_limit; // TODO find a way to const correct
    size_t map_area, map_volume;

    Node **node_map;
    BinaryMinHeap *open_set;

    Node *source_node, *target_node, *current_node;
    size_t current_visit_id = 0;

    // Helper Functions
    u_int8_t hex_to_dec(char hex) const { return (hex <= '9') ? hex - '0' : hex - 'W'; }

    bool set_source_node(Coordinate coordinate) { return (source_node = node_map[coordinate.x + x_limit * (coordinate.y + y_limit * coordinate.z)]); }
    bool set_target_node(Coordinate coordinate) { return (target_node = node_map[coordinate.x + x_limit * (coordinate.y + y_limit * coordinate.z)]); }

    bool set_source_node_reverse(Coordinate coordinate) { return (source_node = node_map[coordinate.x + x_limit * (coordinate.y + y_limit * coordinate.z) - map_area]); }
    bool set_target_node_reverse(Coordinate coordinate) { return (target_node = node_map[coordinate.x + x_limit * (coordinate.y + y_limit * coordinate.z) - map_area]); }

    void set_as_visited(Node *adjacent_node) const;
    void set_inverse_as_visited(Node *adjacent_node) const;

public:
    VoxelGraph(std::istream &stream);
    ~VoxelGraph();

    Path find_path(const Coordinate &source, const Coordinate &target);
    Path find_path_reverse(const Coordinate &source, const Coordinate &target);
};

#endif
