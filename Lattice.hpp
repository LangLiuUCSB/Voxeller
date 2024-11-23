#ifndef LATTICE_HPP
#define LATTICE_HPP

#include "Coordinate.hpp"
#include "Errors.hpp"

#include <iostream>
#include <vector>
#include <map>
#include <string>

class Lattice
{
    size_t x_size, y_size, z_size, area_size, volume_size;

    struct Node;
    struct Arc;
    std::unordered_map<Coordinate, Node *, CoordinateHash> graph; // Node map

    struct SuperNode;
    struct SuperArc;
    std::vector<SuperNode *> condensed_graph; // Supernode List

public:
    Lattice() noexcept = default; // Default constructor

    using FilePath = std::string;
    Lattice(const FilePath &file_path); // todo Parameterized constructor

    Lattice(const Lattice &) noexcept = default;            // Copy constructor
    Lattice(Lattice &&) noexcept = default;                 // Move constructor
    Lattice &operator=(const Lattice &) noexcept = default; // Copy assignment
    Lattice &operator=(Lattice &&) noexcept = default;      // Move assignment

    ~Lattice(); // todo Destructor

    void condense() noexcept; // todo

    using Route = std::string;
    struct TripPlan;
    enum Mode : char;
    Route search(TripPlan trip_plan, Mode search_mode) const; // todo
    // Route super_search(TripPlan trip_plan, Mode supersearch_mode, Mode subsearch_mode) const;
};

struct Lattice::Node
{
    id_t id;
    Coordinate position;
    std::vector<Arc *> outgoings, incomings;
    SuperNode *super;

    Node() noexcept = default;
    Node(const id_t id, const Coordinate position) noexcept : id(id), position(position) {}
    Node(const Node &) noexcept = default;
    Node(Node &&) noexcept = default;
    Node &operator=(const Node &) noexcept = default;
    Node &operator=(Node &&) noexcept = default;
    ~Node() noexcept = default;
};

struct Lattice::Arc
{
    Node *next;
    char move;

    Arc() noexcept = default;
    Arc(Node *next, const char move) noexcept : next(next), move(move) {}
    Arc(const Arc &) noexcept = default;
    Arc(Arc &&) noexcept = default;
    Arc &operator=(const Arc &) noexcept = default;
    Arc &operator=(Arc &&) noexcept = default;
    ~Arc() noexcept = default;
};

struct Lattice::SuperNode
{
    id_t id;
    std::vector<Node *> internals;
    std::vector<struct SuperArc *> outgoings, incomings;

    SuperNode() noexcept = default;
    SuperNode(const id_t id) noexcept : id(id) {}
    SuperNode(const SuperNode &) noexcept = default;
    SuperNode(SuperNode &&) noexcept = default;
    SuperNode &operator=(const SuperNode &) noexcept = default;
    SuperNode &operator=(SuperNode &&) noexcept = default;
    ~SuperNode() noexcept = default;
};

struct Lattice::SuperArc
{
    SuperNode *next;
    Node *exit;
    Arc *entry; // todo: rename

    SuperArc() noexcept = default;
    SuperArc(SuperNode *next, Node *exit, Arc *entry) noexcept
        : next(next), exit(exit), entry(entry) {}
    SuperArc(const SuperArc &) noexcept = default;
    SuperArc(SuperArc &&) noexcept = default;
    SuperArc &operator=(const SuperArc &) noexcept = default;
    SuperArc &operator=(SuperArc &&) noexcept = default;
    ~SuperArc() noexcept = default;
};

struct Lattice::TripPlan
{
    Coordinate source, target;

    TripPlan() noexcept = default;
    TripPlan(const Coordinate source, const Coordinate target) noexcept
        : source(source), target(target) {}
    TripPlan(const TripPlan &) noexcept = default;
    TripPlan(TripPlan &&) noexcept = default;
    TripPlan &operator=(const TripPlan &) noexcept = default;
    TripPlan &operator=(TripPlan &&) noexcept = default;
    ~TripPlan() noexcept = default;
};

enum Lattice::Mode : char
{
    DFS,
    REVERSE_DFS,
    BIDIRECTIONAL_DFS,

    BFS,
    REVERSE_BFS,
    BIDIRECTIONAL_BFS,

    GBFS,
    REVERSE_GBFS,
    BIDIRECTIONAL_GBFS,

    NEGATIVE_GBFS,
    REVERSE_NEGATIVE_GBFS,
    BIDIRECTIONAL_NEGATIVE_GBFS,

    NEGATIVE_A_STAR,
    REVERSE_NEGATIVE_A_STAR,
    BIDIRECTIONAL_NEGATIVE_A_STAR,

    JPS,
    REVERSE_JPS,
    BIDIRECTIONAL_JPS,

    UCS,
    REVERSE_UCS,
    BIDIRECTIONAL_UCS
};

#endif
