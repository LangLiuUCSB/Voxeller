#ifndef LATTICE_HPP
#define LATTICE_HPP

#include "Coordinate.hpp"
#include "TripPlan.hpp"
#include "LatticeErrors.hpp"

#include <iostream>
#include <fstream>

#define LOG std::cout

using FilePath = std::string;
class Lattice
{
public:
    enum SearchMode : char;
    struct Node;
    struct Arc;
    struct SuperNode;
    struct SuperArc;
    using Move = char;
    using Route = std::string;
    using Algorithm = Route (Lattice::*)(Lattice::Node *source, Lattice::Node *target) const;
    using SuperAlgorithm = Route (Lattice::*)(Lattice::Node *source, Lattice::Node *target,
                                              const SearchMode &sub_search_mode) const;

private:
    int x_size, y_size, z_size;
    size_t area_size, volume_size;
    std::unordered_map<Coordinate, Lattice::Node *, CoordinateHash> graph; // Node map
    std::vector<Lattice::SuperNode *> congraph;                            // Supernode List

public:
    Lattice() noexcept = default;                           // Default constructor
    Lattice(const FilePath &file_path);                     // Parameterized constructor
    Lattice(const Lattice &) noexcept = default;            // Copy constructor
    Lattice(Lattice &&) noexcept = default;                 // Move constructor
    Lattice &operator=(const Lattice &) noexcept = default; // Copy assignment
    Lattice &operator=(Lattice &&) noexcept = default;      // Move assignment
    ~Lattice() noexcept;                                    // Default destructor

    size_t node_count() const noexcept { return graph.size(); }
    size_t super_node_count() const noexcept { return congraph.size(); }

    void parse(const FilePath &file_path); // todo handle bad parse
    void condense() noexcept;
    Coordinate travel(const Coordinate &source, const Route &route) const;
    Route search(const TripPlan &trip_plan, const SearchMode &search_mode) const;
    Route super_search(const TripPlan &trip_plan,
                       const SearchMode &super_search_mode,
                       const SearchMode &sub_search_mode) const;

private:
    void tarjan_dfs(Node *u, int visit_time[], int low_link[], bool is_on_stack[],
                    std::stack<Node *> &stack, int &current_time, id_t &id) noexcept;
    Algorithm get_algorithm(const SearchMode &search_mode) const noexcept;
    SuperAlgorithm get_super_algorithm(const SearchMode &search_mode) const noexcept;

    Route dfs(Node *source, Node *target) const;
    Route rdfs(Node *source, Node *target) const;
    Route bddfs(Node *source, Node *target) const;
    Route bfs(Node *source, Node *target) const;
    Route rbfs(Node *source, Node *target) const;
    Route bdbfs(Node *source, Node *target) const;
    Route gbfs(Node *source, Node *target) const;
    Route rgbfs(Node *source, Node *target) const;
    Route bdgbfs(Node *source, Node *target) const;
    Route astar(Node *source, Node *target) const;
    Route rastar(Node *source, Node *target) const;
    Route bdastar(Node *source, Node *target) const;
    Route ngbfs(Node *source, Node *target) const;
    Route rngbfs(Node *source, Node *target) const;
    Route bdngbfs(Node *source, Node *target) const;
    Route nastar(Node *source, Node *target) const;
    Route rnastar(Node *source, Node *target) const;
    Route bdnastar(Node *source, Node *target) const;
    Route jps(Node *source, Node *target) const;
    Route rjps(Node *source, Node *target) const;
    Route bdjps(Node *source, Node *target) const;

    Route super_dfs(Node *source, Node *target, const SearchMode &sub_search_mode) const;
    Route super_rdfs(Node *source, Node *target, const SearchMode &sub_search_mode) const;
    Route super_bddfs(Node *source, Node *target, const SearchMode &sub_search_mode) const;
    Route super_bfs(Node *source, Node *target, const SearchMode &sub_search_mode) const;
    Route super_rbfs(Node *source, Node *target, const SearchMode &sub_search_mode) const;
    Route super_bdbfs(Node *source, Node *target, const SearchMode &sub_search_mode) const;
    Route super_gbfs(Node *source, Node *target, const SearchMode &sub_search_mode) const;
    Route super_rgbfs(Node *source, Node *target, const SearchMode &sub_search_mode) const;
    Route super_bdgbfs(Node *source, Node *target, const SearchMode &sub_search_mode) const;
    Route super_astar(Node *source, Node *target, const SearchMode &sub_search_mode) const;
    Route super_rastar(Node *source, Node *target, const SearchMode &sub_search_mode) const;
    Route super_bdastar(Node *source, Node *target, const SearchMode &sub_search_mode) const;
    Route super_ngbfs(Node *source, Node *target, const SearchMode &sub_search_mode) const;
    Route super_rngbfs(Node *source, Node *target, const SearchMode &sub_search_mode) const;
    Route super_bdngbfs(Node *source, Node *target, const SearchMode &sub_search_mode) const;
    Route super_nastar(Node *source, Node *target, const SearchMode &sub_search_mode) const;
    Route super_rnastar(Node *source, Node *target, const SearchMode &sub_search_mode) const;
    Route super_bdnastar(Node *source, Node *target, const SearchMode &sub_search_mode) const;
    Route super_jps(Node *source, Node *target, const SearchMode &sub_search_mode) const;
    Route super_rjps(Node *source, Node *target, const SearchMode &sub_search_mode) const;
    Route super_bdjps(Node *source, Node *target, const SearchMode &sub_search_mode) const;
};

enum Lattice::SearchMode : char
{
    DFS, // Depth-First Search
    REVERSE_DFS,
    BIDIRECTIONAL_DFS,
    BFS, // Breadth-First Search
    REVERSE_BFS,
    BIDIRECTIONAL_BFS,
    GBFS, // Greedy Best-First Search
    REVERSE_GBFS,
    BIDIRECTIONAL_GBFS,
    A_STAR, // A* Search
    REVERSE_A_STAR,
    BIDIRECTIONAL_A_STAR,
    NEGATIVE_GBFS, // Max-Heap Greedy Best-First Search
    REVERSE_NEGATIVE_GBFS,
    BIDIRECTIONAL_NEGATIVE_GBFS,
    NEGATIVE_A_STAR, // Max-Heap A* Search
    REVERSE_NEGATIVE_A_STAR,
    BIDIRECTIONAL_NEGATIVE_A_STAR,
    JPS, // Jump Point Search
    REVERSE_JPS,
    BIDIRECTIONAL_JPS,
};

#endif
