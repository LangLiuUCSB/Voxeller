#include "Lattice.hpp"

#define LOG std::cout
#define HELP std::cout << "help\n"

struct Lattice::Node
{
    id_t id;
    Coordinate position;
    std::vector<Arc *> outgoings, incomings;
    SuperNode *super;

    Node(const id_t id, const Coordinate position) noexcept
        : id(id), position(position) {}               // Parameterized constructor
    Node() noexcept = default;                        // Default constructor
    Node(const Node &) noexcept = default;            // Copy constructor
    Node(Node &&) noexcept = default;                 // Move constructor
    Node &operator=(const Node &) noexcept = default; // Copy assignment
    Node &operator=(Node &&) noexcept = default;      // Move assignment
    ~Node() noexcept = default;                       // Default destructor
};

struct Lattice::Arc
{
    Node *next;
    Move move;

    Arc(Node *next, const char move) noexcept
        : next(next), move(move) {}                 // Parameterized constructor
    Arc() noexcept = default;                       // Default constructor
    Arc(const Arc &) noexcept = default;            // Copy constructor
    Arc(Arc &&) noexcept = default;                 // Move constructor
    Arc &operator=(const Arc &) noexcept = default; // Copy assignment
    Arc &operator=(Arc &&) noexcept = default;      // Move assignment
    ~Arc() noexcept = default;                      // Default destructor
};

struct Lattice::SuperNode
{
    id_t id;
    std::vector<Node *> internals;
    std::vector<SuperArc *> outgoings, incomings;

    SuperNode(const id_t id) noexcept
        : id(id) {}                                             // Parameterized constructor
    SuperNode() noexcept = default;                             // Default constructor
    SuperNode(const SuperNode &) noexcept = default;            // Copy constructor
    SuperNode(SuperNode &&) noexcept = default;                 // Move constructor
    SuperNode &operator=(const SuperNode &) noexcept = default; // Copy assignment
    SuperNode &operator=(SuperNode &&) noexcept = default;      // Move assignment
    ~SuperNode() noexcept = default;                            // Default destructor
};

struct Lattice::SuperArc
{
    SuperNode *next;
    Node *exit;
    Arc *link;

    SuperArc(SuperNode *next, Node *exit, Arc *link) noexcept
        : next(next), exit(exit), link(link) {}               // Parameterized constructor
    SuperArc() noexcept = default;                            // Default constructor
    SuperArc(const SuperArc &) noexcept = default;            // Copy constructor
    SuperArc(SuperArc &&) noexcept = default;                 // Move constructor
    SuperArc &operator=(const SuperArc &) noexcept = default; // Copy assignment
    SuperArc &operator=(SuperArc &&) noexcept = default;      // Move assignment
    ~SuperArc() noexcept = default;                           // Default destructor
};

#define VOID 7
#define SOLID 4
#define NEW_NODE 2
#define TWO_WAY_NODE 1
#define ONE_WAY_NODE 0
#define UPDATE >>= 1
#define WEST -1
#define NORTH -x_size
#define HAS_ADJACENCY < 4

Lattice::Lattice(const FilePath &file_path) // todo handle bad parse
{
    if (file_path.size() < 4 || file_path.compare(file_path.size() - 4, 4, ".vox") != 0)
        throw std::runtime_error("File at " + file_path + " is not of type .vox");

    // open file
    std::ifstream data(file_path);
    if (data.fail())
        throw std::runtime_error("Could not open " + file_path);

    origin_file_path = file_path;

    // initialize world bounds
    data >> x_size >> y_size >> z_size;
    area_size = x_size * y_size;
    volume_size = area_size * z_size;

    // helpers
    constexpr char bit_mask[] = {0b1000, 0b0100, 0b0010, 0b0001};
    auto hex_to_dec = [](char hex)
    { return (hex <= '9') ? hex - '0' : hex - 'W'; };
    char schematic[area_size];
    std::string row;
    size_t schematic_index = 0;
    for (int y = 0; y < y_size; ++y)
    {
        data >> row;
        for (char hex : row)
            for (char B : bit_mask)
                schematic[schematic_index++] = (hex_to_dec(hex) & B) ? SOLID : VOID;
    }
    auto land = [this](Coordinate position)
    {
        while (graph.find(position) == graph.end())
            position.fall();
        return position;
    };
    auto directed_link = [this](const Coordinate &from, const Coordinate &to, char move)
    {
        graph[from]->outgoings.push_back(new Arc(graph[to], move));
        graph[to]->incomings.push_back(new Arc(graph[from], move));
    };

    // populate graph
    id_t id = 0;
    Coordinate current_position = Coordinate(0, 0, 1);
    for (; current_position.z < z_size; ++current_position.z)
    {
        schematic_index = 0;
        current_position.y = 0;
        for (; current_position.y < y_size; ++current_position.y)
        {
            data >> row;
            current_position.x = 0;
            for (char hex : row)
                for (unsigned char i = 0; i < 4; ++i, ++schematic_index, ++current_position.x)
                {
                    if (hex_to_dec(hex) & bit_mask[i])
                        schematic[schematic_index] = SOLID;
                    else // current voxel is not SOLID
                    {
                        if (schematic[schematic_index] == VOID)
                            continue;
                        schematic[schematic_index] UPDATE;
                        if (schematic[schematic_index] == NEW_NODE)
                        {
                            graph[current_position] = new Lattice::Node(id++, current_position);
                            Coordinate u = current_position;
                            if (current_position.x != 0 &&
                                schematic[schematic_index WEST] HAS_ADJACENCY)
                            {
                                Coordinate v = land(current_position.west());
                                directed_link(u, v, 'w');
                                if (schematic[schematic_index WEST] != ONE_WAY_NODE)
                                    directed_link(v, u, 'e');
                            }
                            if (current_position.y != 0 &&
                                schematic[schematic_index NORTH] HAS_ADJACENCY)
                            {
                                Coordinate v = land(current_position.north());
                                directed_link(u, v, 'n');
                                if (schematic[schematic_index NORTH] != ONE_WAY_NODE)
                                    directed_link(v, u, 's');
                            }
                        }
                        if (schematic[schematic_index] == TWO_WAY_NODE)
                        {
                            Coordinate u = current_position.down();
                            if (current_position.x != 0 &&
                                schematic[schematic_index WEST] == NEW_NODE)
                            {
                                Coordinate v = current_position.west();
                                directed_link(u, v, 'w');
                                directed_link(v, u, 'e');
                            }
                            if (current_position.y != 0 &&
                                schematic[schematic_index NORTH] == NEW_NODE)
                            {
                                Coordinate v = current_position.north();
                                directed_link(u, v, 'n');
                                directed_link(v, u, 's');
                            }
                        }
                        if (schematic[schematic_index] == ONE_WAY_NODE)
                        {
                            Coordinate u = land(current_position.down().down());
                            if (current_position.x != 0 &&
                                schematic[schematic_index WEST] == NEW_NODE)
                            {
                                Coordinate v = current_position.west();
                                directed_link(v, u, 'e');
                            }
                            if (current_position.y != 0 &&
                                schematic[schematic_index NORTH] == NEW_NODE)
                            {
                                Coordinate v = current_position.north();
                                directed_link(v, u, 's');
                            }
                        }
                    }
                }
        }
    }
}

Lattice::~Lattice() noexcept
{
    for (const auto &[position, node] : graph)
    {
        for (const auto &arc : node->outgoings)
            delete arc;
        for (const auto &arc : node->incomings)
            delete arc;
        delete node;
    }
    for (const auto &supernode : congraph)
    {
        for (const auto &superarc : supernode->outgoings)
            delete superarc;
        for (const auto &superarc : supernode->incomings)
            delete superarc;
        delete supernode;
    }
}

Coordinate Lattice::travel(const Coordinate &source, const Route &route) const
{
    if (graph.find(source) == graph.end()) // check source validity
        throw InvalidSource(source);
    Node *current = graph.at(source);
    for (size_t i = 0; i < route.size(); ++i)
    {
        Node *next = nullptr;
        for (Arc *arc : current->outgoings)
        {
            if (arc->move == route[i])
                next = arc->next;
        }
        if (next == nullptr)
            throw InvalidRoute(route[i], i);
        current = next;
    }
    return current->position;
}

void Lattice::condense() noexcept
{
    int *visit_time = new int[graph.size()]();
    int *low_link = new int[graph.size()]();
    bool *is_on_stack = new bool[graph.size()]();
    std::stack<Node *> stack;
    int current_time = 0;
    id_t id = 0;
    for (auto [position, node] : graph)
        if (visit_time[node->id] == 0)
            tarjan_dfs(node, visit_time, low_link, is_on_stack, stack, current_time, id);
    delete[] visit_time;
    delete[] low_link;
    delete[] is_on_stack;
    for (SuperNode *super_node : congraph)
        for (Node *node : super_node->internals)
        {
            for (Arc *arc : node->outgoings)
                if (arc->next->super != super_node)
                    super_node->outgoings.push_back(new SuperArc(arc->next->super, node, arc));
            for (Arc *arc : node->incomings)
                if (arc->next->super != super_node)
                    super_node->incomings.push_back(new SuperArc(arc->next->super, node, arc));
        }
}

void Lattice::tarjan_dfs(Node *u, int visit_time[], int low_link[], bool is_on_stack[],
                         std::stack<Node *> &stack, int &current_time, id_t &id) noexcept
{
    visit_time[u->id] = low_link[u->id] = ++current_time;
    stack.push(u);
    is_on_stack[u->id] = true;
    for (Arc *arc : u->outgoings)
    {
        Node *v = arc->next;
        if (visit_time[v->id] == 0)
        {
            tarjan_dfs(v, visit_time, low_link, is_on_stack, stack, current_time, id);
            low_link[u->id] = std::min(low_link[u->id], low_link[v->id]);
        }
        else if (is_on_stack[v->id] == true)
            low_link[u->id] = std::min(low_link[u->id], visit_time[v->id]);
    }
    if (low_link[u->id] == visit_time[u->id])
    {
        SuperNode *component = new SuperNode(id++);
        Node *current;
        do
        {
            current = stack.top();
            stack.pop();
            is_on_stack[current->id] = false;
            current->super = component;
            component->internals.push_back(current);
        } while (current != u);
        congraph.push_back(component);
    }
}

Lattice::Route Lattice::search(const TripPlan &trip_plan, const SearchMode &search_mode) const
{
    if (graph.find(trip_plan.source) == graph.end()) // check source validity
        throw InvalidSource(trip_plan.source);
    Node *source = graph.at(trip_plan.source);

    if (graph.find(trip_plan.target) == graph.end()) // check target validity
        throw InvalidTarget(trip_plan.target);
    Node *target = graph.at(trip_plan.target);

    Algorithm algorithm = get_algorithm(search_mode);
    if (algorithm == nullptr)
        throw InvalidSearchMode(search_mode);

    try
    {
        return (this->*algorithm)(source, target);
    }
    catch (const std::exception &e)
    {
        throw;
    }
}

Lattice::Route Lattice::super_search(const TripPlan &trip_plan,
                                     const SearchMode &super_search_mode,
                                     const SearchMode &sub_search_mode) const
{
    if (graph.find(trip_plan.source) == graph.end()) // check source validity
        throw InvalidSource(trip_plan.source);
    Node *source = graph.at(trip_plan.source);

    if (graph.find(trip_plan.target) == graph.end()) // check target validity
        throw InvalidTarget(trip_plan.target);
    Node *target = graph.at(trip_plan.target);

    SuperAlgorithm super_algorithm = get_super_algorithm(super_search_mode);
    if (super_algorithm == nullptr)
        throw InvalidSearchMode(super_search_mode);

    if (source->super != target->super)
    {
        std::cout << "SUPER\n";
        try
        {
            return (this->*super_algorithm)(source, target, sub_search_mode);
        }
        catch (const std::exception &e)
        {
            throw;
        }
    }

    Algorithm algorithm = get_algorithm(sub_search_mode);
    if (algorithm == nullptr)
        throw InvalidSearchMode(sub_search_mode);

    try
    {
        return (this->*algorithm)(source, target);
    }
    catch (const std::exception &e)
    {
        throw;
    }
}

Lattice::Algorithm Lattice::get_algorithm(const SearchMode &search_mode) const noexcept
{
    switch (search_mode)
    {
    case DFS:
        return &Lattice::dfs;
    case REVERSE_DFS:
        return &Lattice::rdfs;
    case BIDIRECTIONAL_DFS:
        return &Lattice::bddfs;
    case BFS:
        return &Lattice::bfs;
    case REVERSE_BFS:
        return &Lattice::rbfs;
    case BIDIRECTIONAL_BFS:
        return &Lattice::bdbfs;
    case GBFS:
        return &Lattice::gbfs;
    case REVERSE_GBFS:
        return &Lattice::rgbfs;
    case BIDIRECTIONAL_GBFS:
        return &Lattice::bdgbfs;
    case NEGATIVE_GBFS:
        return &Lattice::ngbfs;
    case REVERSE_NEGATIVE_GBFS:
        return &Lattice::rngbfs;
    case BIDIRECTIONAL_NEGATIVE_GBFS:
        return &Lattice::bdngbfs;
    case A_STAR:
        return &Lattice::astar;
    case REVERSE_A_STAR:
        return &Lattice::rastar;
    case BIDIRECTIONAL_A_STAR:
        return &Lattice::bdastar;
    case NEGATIVE_A_STAR:
        return &Lattice::nastar;
    case REVERSE_NEGATIVE_A_STAR:
        return &Lattice::rnastar;
    case BIDIRECTIONAL_NEGATIVE_A_STAR:
        return &Lattice::bdnastar;
    }
    return nullptr;
}

Lattice::SuperAlgorithm Lattice::get_super_algorithm(const SearchMode &search_mode) const noexcept
{
    switch (search_mode)
    {
    case DFS:
        return &Lattice::super_dfs;
    case REVERSE_DFS:
        return &Lattice::super_rdfs;
    case BIDIRECTIONAL_DFS:
        return &Lattice::super_bddfs;
    case BFS:
        return &Lattice::super_bfs;
    case REVERSE_BFS:
        return &Lattice::super_rbfs;
    case BIDIRECTIONAL_BFS:
        return &Lattice::super_bdbfs;
    case GBFS:
        return &Lattice::super_gbfs;
    case REVERSE_GBFS:
        return &Lattice::super_rgbfs;
    case BIDIRECTIONAL_GBFS:
        return &Lattice::super_bdgbfs;
    case NEGATIVE_GBFS:
        return &Lattice::super_ngbfs;
    case REVERSE_NEGATIVE_GBFS:
        return &Lattice::super_rngbfs;
    case BIDIRECTIONAL_NEGATIVE_GBFS:
        return &Lattice::super_bdngbfs;
    case A_STAR:
        return &Lattice::super_astar;
    case REVERSE_A_STAR:
        return &Lattice::super_rastar;
    case BIDIRECTIONAL_A_STAR:
        return &Lattice::super_bdastar;
    case NEGATIVE_A_STAR:
        return &Lattice::super_nastar;
    case REVERSE_NEGATIVE_A_STAR:
        return &Lattice::super_rnastar;
    case BIDIRECTIONAL_NEGATIVE_A_STAR:
        return &Lattice::super_bdnastar;
    }
    return nullptr;
}

Lattice::Route Lattice::dfs(Node *source, Node *target) const
{
    if (source == target) // trivial case
        return Route();

    Node **last = new Node *[graph.size()]();
    Move *move = new Move[graph.size()]();
    _2Ls::BoxArray<Node *> *open_set = new _2Ls::BoxStack<Node *>(area_size * (z_size / 2));

    for (Node *current = source;;)
    {
        // track current adjacencies
        for (Arc *arc : current->outgoings)
        {
            if (move[arc->next->id])
                continue;
            last[arc->next->id] = current;
            move[arc->next->id] = arc->move;
            open_set->push(arc->next);
        }

        if (open_set->empty())
            break;
        current = open_set->extract();

        if (current == target) // goal check
        {
            Route route;
            // retrace route
            for (; current != source; current = last[current->id])
                route.push_back(move[current->id]);
            std::reverse(route.begin(), route.end());
            delete[] last, delete[] move, delete open_set;
            return route;
        }
    }

    // when no path is found
    delete[] last, delete[] move, delete open_set;
    throw Untraversable(source->position, target->position);
}

Lattice::Route Lattice::rdfs(Node *source, Node *target) const
{
    if (source == target) // trivial case
        return Route();

    Node **last = new Node *[graph.size()]();
    Move *move = new Move[graph.size()]();
    _2Ls::BoxArray<Node *> *open_set = new _2Ls::BoxStack<Node *>(area_size * (z_size / 2));

    for (Node *current = target;;)
    {
        // track current adjacencies
        for (Arc *arc : current->incomings)
        {
            if (move[arc->next->id])
                continue;
            last[arc->next->id] = current;
            move[arc->next->id] = arc->move;
            open_set->push(arc->next);
        }

        if (open_set->empty())
            break;
        current = open_set->extract();

        if (current == source) // goal check
        {
            Route route;
            // retrace route
            for (; current != target; current = last[current->id])
                route.push_back(move[current->id]);
            delete[] last, delete[] move, delete open_set;
            return route;
        }
    }

    // when no path is found
    delete[] last, delete[] move, delete open_set;
    throw Untraversable(source->position, target->position);
}

Lattice::Route Lattice::bddfs(Node *source, Node *target) const
{
    if (source == target) // trivial case
        return Route();

    Node **last_f = new Node *[graph.size()](), **last_b = new Node *[graph.size()]();
    Move *move_f = new Move[graph.size()](), *move_b = new Move[graph.size()]();
    _2Ls::BoxArray<Node *> *open_set_f = new _2Ls::BoxStack<Node *>(area_size * (z_size / 2)),
                           *open_set_b = new _2Ls::BoxStack<Node *>(area_size * (z_size / 2));

    for (Node *current_f = source, *current_b = target;;)
    {
        // track current adjacencies
        for (Arc *arc : current_f->outgoings) // (forwards)
        {
            if (move_f[arc->next->id])
                continue;
            last_f[arc->next->id] = current_f;
            move_f[arc->next->id] = arc->move;
            open_set_f->push(arc->next);
        }
        for (Arc *arc : current_b->incomings) // (backwards)
        {
            if (move_b[arc->next->id])
                continue;
            last_b[arc->next->id] = current_b;
            move_b[arc->next->id] = arc->move;
            open_set_b->push(arc->next);
        }

        if (open_set_f->empty())
            break;
        current_f = open_set_f->extract(); // get next node (forwards)

        if (move_b[current_f->id]) // goal check (forwards)
        {
            Route route;
            current_b = current_f;
            for (; current_f != source; current_f = last_f[current_f->id])
                route.push_back(move_f[current_f->id]);
            std::reverse(route.begin(), route.end());
            for (; current_b != target; current_b = last_b[current_b->id])
                route.push_back(move_b[current_b->id]);
            delete[] last_f, delete[] move_f, delete open_set_f,
                delete[] last_b, delete[] move_b, delete open_set_b;
            return route;
        }

        if (open_set_b->empty())
            break;
        current_b = open_set_b->extract(); // get next node (backwards)

        if (move_f[current_b->id]) // goal check (backwards)
        {
            Route route;
            current_f = current_b;
            for (; current_f != source; current_f = last_f[current_f->id])
                route.push_back(move_f[current_f->id]);
            std::reverse(route.begin(), route.end());
            for (; current_b != target; current_b = last_b[current_b->id])
                route.push_back(move_b[current_b->id]);
            delete[] last_f, delete[] move_f, delete open_set_f,
                delete[] last_b, delete[] move_b, delete open_set_b;
            return route;
        }
    }

    // when no path is found
    delete[] last_f, delete[] move_f, delete open_set_f,
        delete[] last_b, delete[] move_b, delete open_set_b;
    throw Untraversable(source->position, target->position);
}

Lattice::Route Lattice::bfs(Node *source, Node *target) const
{
    if (source == target) // trivial case
        return Route();

    Node **last = new Node *[graph.size()]();
    Move *move = new Move[graph.size()]();
    _2Ls::BoxArray<Node *> *open_set = new _2Ls::BoxQueue<Node *>(area_size * (z_size / 2));

    for (Node *current = source;;)
    {
        // track current adjacencies
        for (Arc *arc : current->outgoings)
        {
            if (move[arc->next->id])
                continue;
            last[arc->next->id] = current;
            move[arc->next->id] = arc->move;
            open_set->push(arc->next);
        }

        if (open_set->empty())
            break;
        current = open_set->extract();

        if (current == target) // goal check
        {
            Route route;
            // retrace route
            for (; current != source; current = last[current->id])
                route.push_back(move[current->id]);
            std::reverse(route.begin(), route.end());
            delete[] last, delete[] move, delete open_set;
            return route;
        }
    }

    // when no path is found
    delete[] last, delete[] move, delete open_set;
    throw Untraversable(source->position, target->position);
}

Lattice::Route Lattice::rbfs(Node *source, Node *target) const
{
    if (source == target) // trivial case
        return Route();

    Node **last = new Node *[graph.size()]();
    Move *move = new Move[graph.size()]();
    _2Ls::BoxArray<Node *> *open_set = new _2Ls::BoxQueue<Node *>(area_size * (z_size / 2));

    for (Node *current = target;;)
    {
        // track current adjacencies
        for (Arc *arc : current->incomings)
        {
            if (move[arc->next->id])
                continue;
            last[arc->next->id] = current;
            move[arc->next->id] = arc->move;
            open_set->push(arc->next);
        }

        if (open_set->empty())
            break;
        current = open_set->extract();

        if (current == source) // goal check
        {
            Route route;
            // retrace route
            for (; current != target; current = last[current->id])
                route.push_back(move[current->id]);
            delete[] last, delete[] move, delete open_set;
            return route;
        }
    }

    // when no path is found
    delete[] last, delete[] move, delete open_set;
    throw Untraversable(source->position, target->position);
}

Lattice::Route Lattice::bdbfs(Node *source, Node *target) const
{
    if (source == target) // trivial case
        return Route();

    Node **last_f = new Node *[graph.size()](), **last_b = new Node *[graph.size()]();
    Move *move_f = new Move[graph.size()](), *move_b = new Move[graph.size()]();
    _2Ls::BoxArray<Node *> *open_set_f = new _2Ls::BoxQueue<Node *>(area_size * (z_size / 2)),
                           *open_set_b = new _2Ls::BoxQueue<Node *>(area_size * (z_size / 2));

    for (Node *current_f = source, *current_b = target;;)
    {
        // track current adjacencies
        for (Arc *arc : current_f->outgoings) // (forwards)
        {
            if (move_f[arc->next->id])
                continue;
            last_f[arc->next->id] = current_f;
            move_f[arc->next->id] = arc->move;
            open_set_f->push(arc->next);
        }
        for (Arc *arc : current_b->incomings) // (backwards)
        {
            if (move_b[arc->next->id])
                continue;
            last_b[arc->next->id] = current_b;
            move_b[arc->next->id] = arc->move;
            open_set_b->push(arc->next);
        }

        if (open_set_f->empty())
            break;
        current_f = open_set_f->extract(); // get next node (forwards)

        if (move_b[current_f->id]) // goal check (forwards)
        {
            Route route;
            current_b = current_f;
            for (; current_f != source; current_f = last_f[current_f->id])
                route.push_back(move_f[current_f->id]);
            std::reverse(route.begin(), route.end());
            for (; current_b != target; current_b = last_b[current_b->id])
                route.push_back(move_b[current_b->id]);
            delete[] last_f, delete[] move_f, delete open_set_f,
                delete[] last_b, delete[] move_b, delete open_set_b;
            return route;
        }

        if (open_set_b->empty())
            break;
        current_b = open_set_b->extract(); // get next node (backwards)

        if (move_f[current_b->id]) // goal check (backwards)
        {
            Route route;
            current_f = current_b;
            for (; current_f != source; current_f = last_f[current_f->id])
                route.push_back(move_f[current_f->id]);
            std::reverse(route.begin(), route.end());
            for (; current_b != target; current_b = last_b[current_b->id])
                route.push_back(move_b[current_b->id]);
            delete[] last_f, delete[] move_f, delete open_set_f,
                delete[] last_b, delete[] move_b, delete open_set_b;
            return route;
        }
    }

    // when no path is found
    delete[] last_f, delete[] move_f, delete open_set_f,
        delete[] last_b, delete[] move_b, delete open_set_b;
    throw Untraversable(source->position, target->position);
}

Lattice::Route Lattice::gbfs(Node *source, Node *target) const
{
    if (source == target) // trivial case
        return "";

    // search meta data
    Node **last = new Node *[graph.size()]();
    Move *move = new Move[graph.size()]();
    auto heuristic = [target](Node *n)
    { return manhattan_distance(target->position, n->position); };
    using Pair = std::pair<Node *, size_t>;
    struct PairMinCmp
    {
        bool operator()(const Pair &a, const Pair &b) { return a.second > b.second; }
    };
    std::priority_queue<Pair, std::vector<Pair>, PairMinCmp> open_set;

    // track initial adjacencies
    for (Arc *arc : source->outgoings)
    {
        last[arc->next->id] = source;
        move[arc->next->id] = arc->move;
        open_set.emplace(arc->next, heuristic(arc->next));
    }

    // search
    while (!open_set.empty())
    {
        auto [current, key] = open_set.top(); // get next node
        open_set.pop();

        if (current == target) // goal check
        {
            Route route;
            for (; current != source; current = last[current->id])
                route.push_back(move[current->id]);
            std::reverse(route.begin(), route.end());
            delete[] last, delete[] move;
            return route;
        }

        // track current adjacencies
        for (Arc *arc : current->outgoings)
        {
            if (move[arc->next->id] != 0)
                continue;
            last[arc->next->id] = current;
            move[arc->next->id] = arc->move;
            open_set.emplace(arc->next, heuristic(arc->next));
        }
    }

    // when no path is found
    delete[] last, delete[] move;
    throw Untraversable(source->position, target->position);
}

Lattice::Route Lattice::rgbfs(Node *source, Node *target) const
{
    if (source == target) // trivial case
        return "";

    // search meta data
    Node **last = new Node *[graph.size()]();
    Move *move = new Move[graph.size()]();
    auto heuristic = [source](Node *n)
    { return manhattan_distance(source->position, n->position); };
    using Pair = std::pair<Node *, size_t>;
    struct PairMinCmp
    {
        bool operator()(const Pair &a, const Pair &b) { return a.second > b.second; }
    };
    std::priority_queue<Pair, std::vector<Pair>, PairMinCmp> open_set;

    // track initial adjacencies
    for (Arc *arc : target->incomings)
    {
        last[arc->next->id] = target;
        move[arc->next->id] = arc->move;
        open_set.emplace(arc->next, heuristic(arc->next));
    }

    // search
    while (!open_set.empty())
    {
        auto [current, key] = open_set.top(); // get next node
        open_set.pop();

        if (current == source) // goal check
        {
            Route route;
            for (; current != target; current = last[current->id])
                route.push_back(move[current->id]);
            delete[] last, delete[] move;
            return route;
        }

        // track current adjacencies
        for (Arc *arc : current->incomings)
        {
            if (move[arc->next->id] != 0)
                continue;
            last[arc->next->id] = current;
            move[arc->next->id] = arc->move;
            open_set.emplace(arc->next, heuristic(arc->next));
        }
    }

    // when no path is found
    delete[] last, delete[] move;
    throw Untraversable(source->position, target->position);
}

Lattice::Route Lattice::bdgbfs(Node *source, Node *target) const
{
    if (source == target) // trivial case
        return "";

    // search meta data
    Node *current_f = source, *current_b = target,
         **last_f = new Node *[graph.size()](), **last_b = new Node *[graph.size()]();
    Move *move_f = new Move[graph.size()](), *move_b = new Move[graph.size()]();
    auto heuristic_f = [&current_b](Node *n)
    { return manhattan_distance(current_b->position, n->position); };
    auto heuristic_b = [&current_f](Node *n)
    { return manhattan_distance(current_f->position, n->position); };
    size_t key_f, key_b;
    using Pair = std::pair<Node *, size_t>;
    struct PairMinCmp
    {
        bool operator()(const Pair &a, const Pair &b) { return a.second > b.second; }
    };
    std::priority_queue<Pair, std::vector<Pair>, PairMinCmp> open_set_f, open_set_b;

    // track initial adjacencies
    for (Arc *arc : source->outgoings) // (forwards)
    {
        last_f[arc->next->id] = source;
        move_f[arc->next->id] = arc->move;
        open_set_f.emplace(arc->next, heuristic_f(arc->next));
    }
    for (Arc *arc : target->incomings) // (backwards)
    {
        last_b[arc->next->id] = target;
        move_b[arc->next->id] = arc->move;
        open_set_b.emplace(arc->next, heuristic_b(arc->next));
    }

    // search
    while (true)
    {
        if (open_set_f.empty())
            break;
        std::tie(current_f, key_f) = open_set_f.top(); // get next node (forwards)
        open_set_f.pop();

        if (move_b[current_f->id]) // goal check (forwards)
        {
            Route route;
            current_b = current_f;
            for (; current_f != source; current_f = last_f[current_f->id])
                route.push_back(move_f[current_f->id]);
            std::reverse(route.begin(), route.end());
            for (; current_b != target; current_b = last_b[current_b->id])
                route.push_back(move_b[current_b->id]);
            delete[] last_f, delete[] move_f;
            delete[] last_b, delete[] move_b;
            return route;
        }

        if (open_set_b.empty())
            break;
        std::tie(current_b, key_b) = open_set_b.top(); // get next node (backwards)
        open_set_b.pop();

        if (move_f[current_b->id]) // goal check (backwards)
        {
            Route route;
            current_f = current_b;
            for (; current_f != source; current_f = last_f[current_f->id])
                route.push_back(move_f[current_f->id]);
            std::reverse(route.begin(), route.end());
            for (; current_b != target; current_b = last_b[current_b->id])
                route.push_back(move_b[current_b->id]);
            delete[] last_f, delete[] move_f;
            delete[] last_b, delete[] move_b;
            return route;
        }

        // track current adjacencies
        for (Arc *arc : current_f->outgoings) // (forwards)
        {
            if (move_f[arc->next->id] != 0)
                continue;
            last_f[arc->next->id] = current_f;
            move_f[arc->next->id] = arc->move;
            open_set_f.emplace(arc->next, heuristic_f(arc->next));
        }
        for (Arc *arc : current_b->incomings) // (backwards)
        {
            if (move_b[arc->next->id] != 0)
                continue;
            last_b[arc->next->id] = current_b;
            move_b[arc->next->id] = arc->move;
            open_set_b.emplace(arc->next, heuristic_b(arc->next));
        }
    }

    // when no path is found
    delete[] last_f, delete[] move_f;
    delete[] last_b, delete[] move_b;
    throw Untraversable(source->position, target->position);
}

Lattice::Route Lattice::ngbfs(Node *source, Node *target) const
{
    if (source == target) // trivial case
        return "";

    // search meta data
    Node **last = new Node *[graph.size()]();
    Move *move = new Move[graph.size()]();
    auto heuristic = [target](Node *n)
    { return manhattan_distance(target->position, n->position); };
    using Pair = std::pair<Node *, size_t>;
    struct PairMaxCmp
    {
        bool operator()(const Pair &a, const Pair &b) { return a.second < b.second; }
    };
    std::priority_queue<Pair, std::vector<Pair>, PairMaxCmp> open_set;

    // track initial adjacencies
    for (Arc *arc : source->outgoings)
    {
        last[arc->next->id] = source;
        move[arc->next->id] = arc->move;
        open_set.emplace(arc->next, heuristic(arc->next));
    }

    // search
    while (!open_set.empty())
    {
        auto [current, key] = open_set.top(); // get next node
        open_set.pop();

        if (current == target) // goal check
        {
            Route route;
            for (; current != source; current = last[current->id])
                route.push_back(move[current->id]);
            std::reverse(route.begin(), route.end());
            delete[] last, delete[] move;
            return route;
        }

        // track current adjacencies
        for (Arc *arc : current->outgoings)
        {
            if (move[arc->next->id] != 0)
                continue;
            last[arc->next->id] = current;
            move[arc->next->id] = arc->move;
            open_set.emplace(arc->next, heuristic(arc->next));
        }
    }

    // when no path is found
    delete[] last, delete[] move;
    throw Untraversable(source->position, target->position);
}

Lattice::Route Lattice::rngbfs(Node *source, Node *target) const
{
    if (source == target) // trivial case
        return "";

    // search meta data
    Node **last = new Node *[graph.size()]();
    Move *move = new Move[graph.size()]();
    auto heuristic = [source](Node *n)
    { return manhattan_distance(source->position, n->position); };
    using Pair = std::pair<Node *, size_t>;
    struct PairMaxCmp
    {
        bool operator()(const Pair &a, const Pair &b) { return a.second < b.second; }
    };
    std::priority_queue<Pair, std::vector<Pair>, PairMaxCmp> open_set;

    // track initial adjacencies
    for (Arc *arc : target->incomings)
    {
        last[arc->next->id] = target;
        move[arc->next->id] = arc->move;
        open_set.emplace(arc->next, heuristic(arc->next));
    }

    // search
    while (!open_set.empty())
    {
        auto [current, key] = open_set.top(); // get next node
        open_set.pop();

        if (current == source) // goal check
        {
            Route route;
            for (; current != target; current = last[current->id])
                route.push_back(move[current->id]);
            delete[] last, delete[] move;
            return route;
        }

        // track current adjacencies
        for (Arc *arc : current->incomings)
        {
            if (move[arc->next->id] != 0)
                continue;
            last[arc->next->id] = current;
            move[arc->next->id] = arc->move;
            open_set.emplace(arc->next, heuristic(arc->next));
        }
    }

    // when no path is found
    delete[] last, delete[] move;
    throw Untraversable(source->position, target->position);
}

Lattice::Route Lattice::bdngbfs(Node *source, Node *target) const
{
    if (source == target) // trivial case
        return "";

    // search meta data
    Node *current_f = source, *current_b = target,
         **last_f = new Node *[graph.size()](), **last_b = new Node *[graph.size()]();
    Move *move_f = new Move[graph.size()](), *move_b = new Move[graph.size()]();
    auto heuristic_f = [&current_b](Node *n)
    { return manhattan_distance(current_b->position, n->position); };
    auto heuristic_b = [&current_f](Node *n)
    { return manhattan_distance(current_f->position, n->position); };
    size_t key_f, key_b;
    using Pair = std::pair<Node *, size_t>;
    struct PairMaxCmp
    {
        bool operator()(const Pair &a, const Pair &b) { return a.second < b.second; }
    };
    std::priority_queue<Pair, std::vector<Pair>, PairMaxCmp> open_set_f, open_set_b;

    // track initial adjacencies
    for (Arc *arc : source->outgoings) // (forwards)
    {
        last_f[arc->next->id] = source;
        move_f[arc->next->id] = arc->move;
        open_set_f.emplace(arc->next, heuristic_f(arc->next));
    }
    for (Arc *arc : target->incomings) // (backwards)
    {
        last_b[arc->next->id] = target;
        move_b[arc->next->id] = arc->move;
        open_set_b.emplace(arc->next, heuristic_b(arc->next));
    }

    // search
    while (true)
    {
        if (open_set_f.empty())
            break;
        std::tie(current_f, key_f) = open_set_f.top(); // get next node (forwards)
        open_set_f.pop();

        if (move_b[current_f->id]) // goal check (forwards)
        {
            Route route;
            current_b = current_f;
            for (; current_f != source; current_f = last_f[current_f->id])
                route.push_back(move_f[current_f->id]);
            std::reverse(route.begin(), route.end());
            for (; current_b != target; current_b = last_b[current_b->id])
                route.push_back(move_b[current_b->id]);
            delete[] last_f, delete[] move_f;
            delete[] last_b, delete[] move_b;
            return route;
        }

        if (open_set_b.empty())
            break;
        std::tie(current_b, key_b) = open_set_b.top(); // get next node (backwards)
        open_set_b.pop();

        if (move_f[current_b->id]) // goal check (backwards)
        {
            Route route;
            current_f = current_b;
            for (; current_f != source; current_f = last_f[current_f->id])
                route.push_back(move_f[current_f->id]);
            std::reverse(route.begin(), route.end());
            for (; current_b != target; current_b = last_b[current_b->id])
                route.push_back(move_b[current_b->id]);
            delete[] last_f, delete[] move_f;
            delete[] last_b, delete[] move_b;
            return route;
        }

        // track current adjacencies
        for (Arc *arc : current_f->outgoings) // (forwards)
        {
            if (move_f[arc->next->id] != 0)
                continue;
            last_f[arc->next->id] = current_f;
            move_f[arc->next->id] = arc->move;
            open_set_f.emplace(arc->next, heuristic_f(arc->next));
        }
        for (Arc *arc : current_b->incomings) // (backwards)
        {
            if (move_b[arc->next->id] != 0)
                continue;
            last_b[arc->next->id] = current_b;
            move_b[arc->next->id] = arc->move;
            open_set_b.emplace(arc->next, heuristic_b(arc->next));
        }
    }

    // when no path is found
    delete[] last_f, delete[] move_f;
    delete[] last_b, delete[] move_b;
    throw Untraversable(source->position, target->position);
}

Lattice::Route Lattice::astar(Node *source, Node *target) const
{
    if (source == target) // trivial case
        return "";

    // search meta data
    Node **last = new Node *[graph.size()]();
    Move *move = new Move[graph.size()]();
    auto heuristic = [source, target](Node *n)
    { return manhattan_distance(source->position, n->position) +
             manhattan_distance(target->position, n->position); };
    using Pair = std::pair<Node *, size_t>;
    struct PairMinCmp
    {
        bool operator()(const Pair &a, const Pair &b) { return a.second > b.second; }
    };
    std::priority_queue<Pair, std::vector<Pair>, PairMinCmp> open_set;

    // track initial adjacencies
    for (Arc *arc : source->outgoings)
    {
        last[arc->next->id] = source;
        move[arc->next->id] = arc->move;
        open_set.emplace(arc->next, heuristic(arc->next));
    }

    // search
    while (!open_set.empty())
    {
        auto [current, key] = open_set.top(); // get next node
        open_set.pop();

        if (current == target) // goal check
        {
            Route route;
            for (; current != source; current = last[current->id])
                route.push_back(move[current->id]);
            std::reverse(route.begin(), route.end());
            delete[] last, delete[] move;
            return route;
        }

        // track current adjacencies
        for (Arc *arc : current->outgoings)
        {
            if (move[arc->next->id] != 0)
                continue;
            last[arc->next->id] = current;
            move[arc->next->id] = arc->move;
            open_set.emplace(arc->next, heuristic(arc->next));
        }
    }

    // when no path is found
    delete[] last, delete[] move;
    throw Untraversable(source->position, target->position);
}

Lattice::Route Lattice::rastar(Node *source, Node *target) const
{
    if (source == target) // trivial case
        return "";

    // search meta data
    Node **last = new Node *[graph.size()]();
    Move *move = new Move[graph.size()]();
    auto heuristic = [target, source](Node *n)
    { return manhattan_distance(target->position, n->position) +
             manhattan_distance(source->position, n->position); };
    using Pair = std::pair<Node *, size_t>;
    struct PairMinCmp
    {
        bool operator()(const Pair &a, const Pair &b) { return a.second > b.second; }
    };
    std::priority_queue<Pair, std::vector<Pair>, PairMinCmp> open_set;

    // track initial adjacencies
    for (Arc *arc : target->incomings)
    {
        last[arc->next->id] = target;
        move[arc->next->id] = arc->move;
        open_set.emplace(arc->next, heuristic(arc->next));
    }

    // search
    while (!open_set.empty())
    {
        auto [current, key] = open_set.top(); // get next node
        open_set.pop();

        if (current == source) // goal check
        {
            Route route;
            for (; current != target; current = last[current->id])
                route.push_back(move[current->id]);
            delete[] last, delete[] move;
            return route;
        }

        // track current adjacencies
        for (Arc *arc : current->incomings)
        {
            if (move[arc->next->id] != 0)
                continue;
            last[arc->next->id] = current;
            move[arc->next->id] = arc->move;
            open_set.emplace(arc->next, heuristic(arc->next));
        }
    }

    // when no path is found
    delete[] last, delete[] move;
    throw Untraversable(source->position, target->position);
}

Lattice::Route Lattice::bdastar(Node *source, Node *target) const
{
    if (source == target) // trivial case
        return "";

    // search meta data
    Node *current_f = source, *current_b = target,
         **last_f = new Node *[graph.size()](), **last_b = new Node *[graph.size()]();
    Move *move_f = new Move[graph.size()](), *move_b = new Move[graph.size()]();
    auto heuristic_f = [source, &current_b](Node *n)
    { return manhattan_distance(source->position, n->position) +
             manhattan_distance(current_b->position, n->position); };
    auto heuristic_b = [target, &current_f](Node *n)
    { return manhattan_distance(target->position, n->position) +
             manhattan_distance(current_f->position, n->position); };
    size_t key_f, key_b;
    using Pair = std::pair<Node *, size_t>;
    struct PairMinCmp
    {
        bool operator()(const Pair &a, const Pair &b) { return a.second > b.second; }
    };
    std::priority_queue<Pair, std::vector<Pair>, PairMinCmp> open_set_f, open_set_b;

    // track initial adjacencies
    for (Arc *arc : source->outgoings) // (forwards)
    {
        last_f[arc->next->id] = source;
        move_f[arc->next->id] = arc->move;
        open_set_f.emplace(arc->next, heuristic_f(arc->next));
    }
    for (Arc *arc : target->incomings) // (backwards)
    {
        last_b[arc->next->id] = target;
        move_b[arc->next->id] = arc->move;
        open_set_b.emplace(arc->next, heuristic_b(arc->next));
    }

    // search
    while (true)
    {
        if (open_set_f.empty())
            break;
        std::tie(current_f, key_f) = open_set_f.top(); // get next node (forwards)
        open_set_f.pop();

        if (move_b[current_f->id]) // goal check (forwards)
        {
            Route route;
            current_b = current_f;
            for (; current_f != source; current_f = last_f[current_f->id])
                route.push_back(move_f[current_f->id]);
            std::reverse(route.begin(), route.end());
            for (; current_b != target; current_b = last_b[current_b->id])
                route.push_back(move_b[current_b->id]);
            delete[] last_f, delete[] move_f;
            delete[] last_b, delete[] move_b;
            return route;
        }

        if (open_set_b.empty())
            break;
        std::tie(current_b, key_b) = open_set_b.top(); // get next node (backwards)
        open_set_b.pop();

        if (move_f[current_b->id]) // goal check (backwards)
        {
            Route route;
            current_f = current_b;
            for (; current_f != source; current_f = last_f[current_f->id])
                route.push_back(move_f[current_f->id]);
            std::reverse(route.begin(), route.end());
            for (; current_b != target; current_b = last_b[current_b->id])
                route.push_back(move_b[current_b->id]);
            delete[] last_f, delete[] move_f;
            delete[] last_b, delete[] move_b;
            return route;
        }

        // track current adjacencies
        for (Arc *arc : current_f->outgoings) // (forwards)
        {
            if (move_f[arc->next->id] != 0)
                continue;
            last_f[arc->next->id] = current_f;
            move_f[arc->next->id] = arc->move;
            open_set_f.emplace(arc->next, heuristic_f(arc->next));
        }
        for (Arc *arc : current_b->incomings) // (backwards)
        {
            if (move_b[arc->next->id] != 0)
                continue;
            last_b[arc->next->id] = current_b;
            move_b[arc->next->id] = arc->move;
            open_set_b.emplace(arc->next, heuristic_b(arc->next));
        }
    }

    // when no path is found
    delete[] last_f, delete[] move_f;
    delete[] last_b, delete[] move_b;
    throw Untraversable(source->position, target->position);
}

Lattice::Route Lattice::nastar(Node *source, Node *target) const
{
    if (source == target) // trivial case
        return "";

    // search meta data
    Node **last = new Node *[graph.size()]();
    Move *move = new Move[graph.size()]();
    auto heuristic = [source, target](Node *n)
    { return manhattan_distance(source->position, n->position) +
             manhattan_distance(target->position, n->position); };
    using Pair = std::pair<Node *, size_t>;
    struct PairMaxCmp
    {
        bool operator()(const Pair &a, const Pair &b) { return a.second < b.second; }
    };
    std::priority_queue<Pair, std::vector<Pair>, PairMaxCmp> open_set;

    // track initial adjacencies
    for (Arc *arc : source->outgoings)
    {
        last[arc->next->id] = source;
        move[arc->next->id] = arc->move;
        open_set.emplace(arc->next, heuristic(arc->next));
    }

    // search
    while (!open_set.empty())
    {
        auto [current, key] = open_set.top(); // get next node
        open_set.pop();

        if (current == target) // goal check
        {
            Route route;
            for (; current != source; current = last[current->id])
                route.push_back(move[current->id]);
            std::reverse(route.begin(), route.end());
            delete[] last, delete[] move;
            return route;
        }

        // track current adjacencies
        for (Arc *arc : current->outgoings)
        {
            if (move[arc->next->id] != 0)
                continue;
            last[arc->next->id] = current;
            move[arc->next->id] = arc->move;
            open_set.emplace(arc->next, heuristic(arc->next));
        }
    }

    // when no path is found
    delete[] last, delete[] move;
    throw Untraversable(source->position, target->position);
}

Lattice::Route Lattice::rnastar(Node *source, Node *target) const
{
    if (source == target) // trivial case
        return "";

    // search meta data
    Node **last = new Node *[graph.size()]();
    Move *move = new Move[graph.size()]();
    auto heuristic = [target, source](Node *n)
    { return manhattan_distance(target->position, n->position) +
             manhattan_distance(source->position, n->position); };
    using Pair = std::pair<Node *, size_t>;
    struct PairMaxCmp
    {
        bool operator()(const Pair &a, const Pair &b) { return a.second < b.second; }
    };
    std::priority_queue<Pair, std::vector<Pair>, PairMaxCmp> open_set;

    // track initial adjacencies
    for (Arc *arc : target->incomings)
    {
        last[arc->next->id] = target;
        move[arc->next->id] = arc->move;
        open_set.emplace(arc->next, heuristic(arc->next));
    }

    // search
    while (!open_set.empty())
    {
        auto [current, key] = open_set.top(); // get next node
        open_set.pop();

        if (current == source) // goal check
        {
            Route route;
            for (; current != target; current = last[current->id])
                route.push_back(move[current->id]);
            delete[] last, delete[] move;
            return route;
        }

        // track current adjacencies
        for (Arc *arc : current->incomings)
        {
            if (move[arc->next->id] != 0)
                continue;
            last[arc->next->id] = current;
            move[arc->next->id] = arc->move;
            open_set.emplace(arc->next, heuristic(arc->next));
        }
    }

    // when no path is found
    delete[] last, delete[] move;
    throw Untraversable(source->position, target->position);
}

Lattice::Route Lattice::bdnastar(Node *source, Node *target) const
{
    if (source == target) // trivial case
        return "";

    // search meta data
    Node *current_f = source, *current_b = target,
         **last_f = new Node *[graph.size()](), **last_b = new Node *[graph.size()]();
    Move *move_f = new Move[graph.size()](), *move_b = new Move[graph.size()]();
    auto heuristic_f = [source, &current_b](Node *n)
    { return manhattan_distance(source->position, n->position) +
             manhattan_distance(current_b->position, n->position); };
    auto heuristic_b = [target, &current_f](Node *n)
    { return manhattan_distance(target->position, n->position) +
             manhattan_distance(current_f->position, n->position); };
    size_t key_f, key_b;
    using Pair = std::pair<Node *, size_t>;
    struct PairMaxCmp
    {
        bool operator()(const Pair &a, const Pair &b) { return a.second < b.second; }
    };
    std::priority_queue<Pair, std::vector<Pair>, PairMaxCmp> open_set_f, open_set_b;

    // track initial adjacencies
    for (Arc *arc : source->outgoings) // (forwards)
    {
        last_f[arc->next->id] = source;
        move_f[arc->next->id] = arc->move;
        open_set_f.emplace(arc->next, heuristic_f(arc->next));
    }
    for (Arc *arc : target->incomings) // (backwards)
    {
        last_b[arc->next->id] = target;
        move_b[arc->next->id] = arc->move;
        open_set_b.emplace(arc->next, heuristic_b(arc->next));
    }

    // search
    while (true)
    {
        if (open_set_f.empty())
            break;
        std::tie(current_f, key_f) = open_set_f.top(); // get next node (forwards)
        open_set_f.pop();

        if (move_b[current_f->id]) // goal check (forwards)
        {
            Route route;
            current_b = current_f;
            for (; current_f != source; current_f = last_f[current_f->id])
                route.push_back(move_f[current_f->id]);
            std::reverse(route.begin(), route.end());
            for (; current_b != target; current_b = last_b[current_b->id])
                route.push_back(move_b[current_b->id]);
            delete[] last_f, delete[] move_f;
            delete[] last_b, delete[] move_b;
            return route;
        }

        if (open_set_b.empty())
            break;
        std::tie(current_b, key_b) = open_set_b.top(); // get next node (backwards)
        open_set_b.pop();

        if (move_f[current_b->id]) // goal check (backwards)
        {
            Route route;
            current_f = current_b;
            for (; current_f != source; current_f = last_f[current_f->id])
                route.push_back(move_f[current_f->id]);
            std::reverse(route.begin(), route.end());
            for (; current_b != target; current_b = last_b[current_b->id])
                route.push_back(move_b[current_b->id]);
            delete[] last_f, delete[] move_f;
            delete[] last_b, delete[] move_b;
            return route;
        }

        // track current adjacencies
        for (Arc *arc : current_f->outgoings) // (forwards)
        {
            if (move_f[arc->next->id] != 0)
                continue;
            last_f[arc->next->id] = current_f;
            move_f[arc->next->id] = arc->move;
            open_set_f.emplace(arc->next, heuristic_f(arc->next));
        }
        for (Arc *arc : current_b->incomings) // (backwards)
        {
            if (move_b[arc->next->id] != 0)
                continue;
            last_b[arc->next->id] = current_b;
            move_b[arc->next->id] = arc->move;
            open_set_b.emplace(arc->next, heuristic_b(arc->next));
        }
    }

    // when no path is found
    delete[] last_f, delete[] move_f;
    delete[] last_b, delete[] move_b;
    throw Untraversable(source->position, target->position);
}

Lattice::Route Lattice::super_dfs(Node *source, Node *target, const SearchMode &sub_search_mode) const
{
    // search meta data
    SuperNode *super_source = source->super, *super_target = target->super,
              **last = new SuperNode *[congraph.size()]();
    Node **exit = new Node *[congraph.size()](),
         **entry = new Node *[congraph.size()]();
    Move *move = new Move[congraph.size()]();
    std::stack<SuperNode *> open_set;

    // track initial adjacencies
    for (SuperArc *super_arc : super_source->outgoings)
    {
        last[super_arc->next->id] = super_source;
        exit[super_arc->next->id] = super_arc->exit;
        entry[super_arc->next->id] = super_arc->link->next;
        move[super_arc->next->id] = super_arc->link->move;
        open_set.push(super_arc->next);
    }

    // search
    while (!open_set.empty())
    {
        SuperNode *super_current = open_set.top(); // get next node
        open_set.pop();

        if (super_current == super_target) // goal check
        {
            Algorithm algorithm = get_algorithm(sub_search_mode);
            if (algorithm == nullptr)
                throw InvalidSearchMode(sub_search_mode);
            entry[super_source->id] = source;
            Route route = (this->*algorithm)(entry[super_current->id], target);
            do
            {
                route.insert(0, 1, move[super_current->id]);
                Node *temp_exit = exit[super_current->id];
                super_current = last[super_current->id];
                route.insert(0, (this->*algorithm)(entry[super_current->id], temp_exit));
            } while (super_current != super_source);
            delete[] last, delete[] exit, delete[] entry, delete[] move;
            return route;
        }

        // track current adjacencies
        for (SuperArc *super_arc : super_current->outgoings)
        {
            if (move[super_arc->next->id] != 0)
                continue;
            last[super_arc->next->id] = super_current;
            exit[super_arc->next->id] = super_arc->exit;
            entry[super_arc->next->id] = super_arc->link->next;
            move[super_arc->next->id] = super_arc->link->move;
            open_set.push(super_arc->next);
        }
    }

    // when no path is found
    delete[] last, delete[] exit, delete[] entry, delete[] move;
    throw Untraversable(source->position, target->position);
}

Lattice::Route Lattice::super_rdfs(Node *source, Node *target, const SearchMode &sub_search_mode) const
{
    // search meta data
    SuperNode *super_source = source->super, *super_target = target->super,
              **last = new SuperNode *[congraph.size()]();
    Node **exit = new Node *[congraph.size()](),
         **entry = new Node *[congraph.size()]();
    Move *move = new Move[congraph.size()]();
    std::stack<SuperNode *> open_set;

    // track initial adjacencies
    for (SuperArc *super_arc : super_target->incomings)
    {
        last[super_arc->next->id] = super_target;
        entry[super_arc->next->id] = super_arc->exit;
        exit[super_arc->next->id] = super_arc->link->next;
        move[super_arc->next->id] = super_arc->link->move;
        open_set.push(super_arc->next);
    }

    // search
    while (!open_set.empty())
    {
        SuperNode *super_current = open_set.top(); // get next node
        open_set.pop();

        if (super_current == super_source) // goal check
        {
            Algorithm algorithm = get_algorithm(sub_search_mode);
            if (algorithm == nullptr)
                throw InvalidSearchMode(sub_search_mode);
            exit[super_target->id] = target;
            Route route = (this->*algorithm)(source, exit[super_current->id]);
            do
            {
                route += move[super_current->id];
                Node *temp_entry = entry[super_current->id];
                super_current = last[super_current->id];
                route += (this->*algorithm)(temp_entry, exit[super_current->id]);
            } while (super_current != super_target);
            delete[] last, delete[] exit, delete[] entry, delete[] move;
            return route;
        }

        // track current adjacencies
        for (SuperArc *super_arc : super_current->incomings)
        {
            if (move[super_arc->next->id] != 0)
                continue;
            last[super_arc->next->id] = super_current;
            entry[super_arc->next->id] = super_arc->exit;
            exit[super_arc->next->id] = super_arc->link->next;
            move[super_arc->next->id] = super_arc->link->move;
            open_set.push(super_arc->next);
        }
    }

    // when no path is found
    delete[] last, delete[] exit, delete[] entry, delete[] move;
    throw Untraversable(source->position, target->position);
}

Lattice::Route Lattice::super_bddfs(Node *source, Node *target, const SearchMode &sub_search_mode) const
{
    // search meta data
    SuperNode *super_source = source->super, *super_current_f = super_source,
              *super_target = target->super, *super_current_b = super_target,
              **last_f = new SuperNode *[congraph.size()](),
              **last_b = new SuperNode *[congraph.size()]();
    Node **exit_f = new Node *[congraph.size()](), **exit_b = new Node *[congraph.size()](),
         **entry_f = new Node *[congraph.size()](), **entry_b = new Node *[congraph.size()]();
    entry_f[super_source->id] = source, exit_b[super_target->id] = target;
    Move *move_f = new Move[congraph.size()](), *move_b = new Move[congraph.size()]();
    std::stack<SuperNode *> open_set_f, open_set_b;

    // track initial adjacencies
    for (SuperArc *super_arc : super_source->outgoings) // (forwards)
    {
        last_f[super_arc->next->id] = super_source;
        exit_f[super_arc->next->id] = super_arc->exit;
        entry_f[super_arc->next->id] = super_arc->link->next;
        move_f[super_arc->next->id] = super_arc->link->move;
        open_set_f.push(super_arc->next);
    }
    for (SuperArc *super_arc : super_target->incomings) // (backwards)
    {
        last_b[super_arc->next->id] = super_target;
        entry_b[super_arc->next->id] = super_arc->exit;
        exit_b[super_arc->next->id] = super_arc->link->next;
        move_b[super_arc->next->id] = super_arc->link->move;
        open_set_b.push(super_arc->next);
    }

    // search
    while (true)
    {
        if (open_set_f.empty())
            break;

        super_current_f = open_set_f.top(); // get next super node (forwards)
        open_set_f.pop();

        if (entry_b[super_current_f->id] != nullptr) // goal check (forwards)
        {
            Algorithm algorithm = get_algorithm(sub_search_mode);
            if (algorithm == nullptr)
                throw InvalidSearchMode(sub_search_mode);

            super_current_b = super_current_f;
            Route route = (this->*algorithm)(entry_f[super_current_f->id], exit_b[super_current_f->id]);
            do
            {
                route.insert(0, 1, move_f[super_current_f->id]);
                Node *temp_exit = exit_f[super_current_f->id];
                super_current_f = last_f[super_current_f->id];
                route.insert(0, (this->*algorithm)(entry_f[super_current_f->id], temp_exit));
            } while (super_current_f != super_source);
            if (super_current_b != super_target)
                do
                {
                    route += move_b[super_current_b->id];
                    Node *temp_entry = entry_b[super_current_b->id];
                    super_current_b = last_b[super_current_b->id];
                    route += (this->*algorithm)(temp_entry, exit_b[super_current_b->id]);
                } while (super_current_b != super_target);
            delete[] last_f, delete[] exit_f, delete[] entry_f, delete[] move_f,
                delete[] last_b, delete[] exit_b, delete[] entry_b, delete[] move_b;
            return route;
        }

        if (open_set_b.empty())
            break;
        super_current_b = open_set_b.top(); // get next super node (backwards)
        open_set_b.pop();

        if (exit_f[super_current_b->id] != nullptr) // goal check (backwards)
        {
            Algorithm algorithm = get_algorithm(sub_search_mode);
            if (algorithm == nullptr)
                throw InvalidSearchMode(sub_search_mode);

            super_current_f = super_current_b;
            Route route = (this->*algorithm)(entry_f[super_current_b->id], exit_b[super_current_b->id]);
            do
            {
                route += move_b[super_current_b->id];
                Node *temp_entry = entry_b[super_current_b->id];
                super_current_b = last_b[super_current_b->id];
                route += (this->*algorithm)(temp_entry, exit_b[super_current_b->id]);
            } while (super_current_b != super_target);
            if (super_current_b != super_source)
                do
                {
                    route.insert(0, 1, move_f[super_current_f->id]);
                    Node *temp_exit = exit_f[super_current_f->id];
                    super_current_f = last_f[super_current_f->id];
                    route.insert(0, (this->*algorithm)(entry_f[super_current_f->id], temp_exit));
                } while (super_current_f != super_source);
            delete[] last_f, delete[] exit_f, delete[] entry_f, delete[] move_f,
                delete[] last_b, delete[] exit_b, delete[] entry_b, delete[] move_b;
            return route;
        }

        // track current adjacencies
        for (SuperArc *super_arc : super_current_f->outgoings) // (forwards)
        {
            if (move_f[super_arc->next->id] != 0)
                continue;
            last_f[super_arc->next->id] = super_current_f;
            exit_f[super_arc->next->id] = super_arc->exit;
            entry_f[super_arc->next->id] = super_arc->link->next;
            move_f[super_arc->next->id] = super_arc->link->move;
            open_set_f.push(super_arc->next);
        }
        for (SuperArc *super_arc : super_current_b->incomings) // (backwards)
        {
            if (move_b[super_arc->next->id] != 0)
                continue;
            last_b[super_arc->next->id] = super_current_b;
            entry_b[super_arc->next->id] = super_arc->exit;
            exit_b[super_arc->next->id] = super_arc->link->next;
            move_b[super_arc->next->id] = super_arc->link->move;
            open_set_b.push(super_arc->next);
        }
    }

    // when no path is found
    delete[] last_f, delete[] exit_f, delete[] entry_f, delete[] move_f,
        delete[] last_b, delete[] exit_b, delete[] entry_b, delete[] move_b;
    throw Untraversable(source->position, target->position);
}

Lattice::Route Lattice::super_bfs(Node *source, Node *target, const SearchMode &sub_search_mode) const
{
    // search meta data
    SuperNode *super_source = source->super, *super_target = target->super,
              **last = new SuperNode *[congraph.size()]();
    Node **exit = new Node *[congraph.size()](),
         **entry = new Node *[congraph.size()]();
    Move *move = new Move[congraph.size()]();
    std::queue<SuperNode *> open_set;

    // track initial adjacencies
    for (SuperArc *super_arc : super_source->outgoings)
    {
        last[super_arc->next->id] = super_source;
        exit[super_arc->next->id] = super_arc->exit;
        entry[super_arc->next->id] = super_arc->link->next;
        move[super_arc->next->id] = super_arc->link->move;
        open_set.push(super_arc->next);
    }

    // search
    while (!open_set.empty())
    {
        SuperNode *super_current = open_set.front(); // get next node
        open_set.pop();

        if (super_current == super_target) // goal check
        {
            Algorithm algorithm = get_algorithm(sub_search_mode);
            if (algorithm == nullptr)
                throw InvalidSearchMode(sub_search_mode);
            entry[super_source->id] = source;
            Route route = (this->*algorithm)(entry[super_current->id], target);
            do
            {
                route.insert(0, 1, move[super_current->id]);
                Node *temp_exit = exit[super_current->id];
                super_current = last[super_current->id];
                route.insert(0, (this->*algorithm)(entry[super_current->id], temp_exit));
            } while (super_current != super_source);
            delete[] last, delete[] exit, delete[] entry, delete[] move;
            return route;
        }

        // track current adjacencies
        for (SuperArc *super_arc : super_current->outgoings)
        {
            if (move[super_arc->next->id] != 0)
                continue;
            last[super_arc->next->id] = super_current;
            exit[super_arc->next->id] = super_arc->exit;
            entry[super_arc->next->id] = super_arc->link->next;
            move[super_arc->next->id] = super_arc->link->move;
            open_set.push(super_arc->next);
        }
    }

    // when no path is found
    delete[] last, delete[] exit, delete[] entry, delete[] move;
    throw Untraversable(source->position, target->position);
}

Lattice::Route Lattice::super_rbfs(Node *source, Node *target, const SearchMode &sub_search_mode) const
{
    // search meta data
    SuperNode *super_source = source->super, *super_target = target->super,
              **last = new SuperNode *[congraph.size()]();
    Node **exit = new Node *[congraph.size()](),
         **entry = new Node *[congraph.size()]();
    Move *move = new Move[congraph.size()]();
    std::queue<SuperNode *> open_set;

    // track initial adjacencies
    for (SuperArc *super_arc : super_target->incomings)
    {
        last[super_arc->next->id] = super_target;
        entry[super_arc->next->id] = super_arc->exit;
        exit[super_arc->next->id] = super_arc->link->next;
        move[super_arc->next->id] = super_arc->link->move;
        open_set.push(super_arc->next);
    }

    // search
    while (!open_set.empty())
    {
        SuperNode *super_current = open_set.front(); // get next node
        open_set.pop();

        if (super_current == super_source) // goal check
        {
            Algorithm algorithm = get_algorithm(sub_search_mode);
            if (algorithm == nullptr)
                throw InvalidSearchMode(sub_search_mode);
            exit[super_target->id] = target;
            Route route = (this->*algorithm)(source, exit[super_current->id]);
            do
            {
                route += move[super_current->id];
                Node *temp_entry = entry[super_current->id];
                super_current = last[super_current->id];
                route += (this->*algorithm)(temp_entry, exit[super_current->id]);
            } while (super_current != super_target);
            delete[] last, delete[] exit, delete[] entry, delete[] move;
            return route;
        }

        // track current adjacencies
        for (SuperArc *super_arc : super_current->incomings)
        {
            if (move[super_arc->next->id] != 0)
                continue;
            last[super_arc->next->id] = super_current;
            entry[super_arc->next->id] = super_arc->exit;
            exit[super_arc->next->id] = super_arc->link->next;
            move[super_arc->next->id] = super_arc->link->move;
            open_set.push(super_arc->next);
        }
    }

    // when no path is found
    delete[] last, delete[] exit, delete[] entry, delete[] move;
    throw Untraversable(source->position, target->position);
}

Lattice::Route Lattice::super_bdbfs(Node *source, Node *target, const SearchMode &sub_search_mode) const
{
    // search meta data
    SuperNode *super_source = source->super, *super_current_f = super_source,
              *super_target = target->super, *super_current_b = super_target,
              **last_f = new SuperNode *[congraph.size()](),
              **last_b = new SuperNode *[congraph.size()]();
    Node **exit_f = new Node *[congraph.size()](), **exit_b = new Node *[congraph.size()](),
         **entry_f = new Node *[congraph.size()](), **entry_b = new Node *[congraph.size()]();
    entry_f[super_source->id] = source, exit_b[super_target->id] = target;
    Move *move_f = new Move[congraph.size()](), *move_b = new Move[congraph.size()]();
    std::queue<SuperNode *> open_set_f, open_set_b;

    // track initial adjacencies
    for (SuperArc *super_arc : super_source->outgoings) // (forwards)
    {
        last_f[super_arc->next->id] = super_source;
        exit_f[super_arc->next->id] = super_arc->exit;
        entry_f[super_arc->next->id] = super_arc->link->next;
        move_f[super_arc->next->id] = super_arc->link->move;
        open_set_f.push(super_arc->next);
    }
    for (SuperArc *super_arc : super_target->incomings) // (backwards)
    {
        last_b[super_arc->next->id] = super_target;
        entry_b[super_arc->next->id] = super_arc->exit;
        exit_b[super_arc->next->id] = super_arc->link->next;
        move_b[super_arc->next->id] = super_arc->link->move;
        open_set_b.push(super_arc->next);
    }

    // search
    while (true)
    {
        if (open_set_f.empty())
            break;

        super_current_f = open_set_f.front(); // get next super node (forwards)
        open_set_f.pop();

        if (entry_b[super_current_f->id] != nullptr) // goal check (forwards)
        {
            Algorithm algorithm = get_algorithm(sub_search_mode);
            if (algorithm == nullptr)
                throw InvalidSearchMode(sub_search_mode);

            super_current_b = super_current_f;
            Route route = (this->*algorithm)(entry_f[super_current_f->id], exit_b[super_current_f->id]);
            do
            {
                route.insert(0, 1, move_f[super_current_f->id]);
                Node *temp_exit = exit_f[super_current_f->id];
                super_current_f = last_f[super_current_f->id];
                route.insert(0, (this->*algorithm)(entry_f[super_current_f->id], temp_exit));
            } while (super_current_f != super_source);
            if (super_current_b != super_target)
                do
                {
                    route += move_b[super_current_b->id];
                    Node *temp_entry = entry_b[super_current_b->id];
                    super_current_b = last_b[super_current_b->id];
                    route += (this->*algorithm)(temp_entry, exit_b[super_current_b->id]);
                } while (super_current_b != super_target);
            delete[] last_f, delete[] exit_f, delete[] entry_f, delete[] move_f,
                delete[] last_b, delete[] exit_b, delete[] entry_b, delete[] move_b;
            return route;
        }

        if (open_set_b.empty())
            break;
        super_current_b = open_set_b.front(); // get next super node (backwards)
        open_set_b.pop();

        if (exit_f[super_current_b->id] != nullptr) // goal check (backwards)
        {
            Algorithm algorithm = get_algorithm(sub_search_mode);
            if (algorithm == nullptr)
                throw InvalidSearchMode(sub_search_mode);

            super_current_f = super_current_b;
            Route route = (this->*algorithm)(entry_f[super_current_b->id], exit_b[super_current_b->id]);
            do
            {
                route += move_b[super_current_b->id];
                Node *temp_entry = entry_b[super_current_b->id];
                super_current_b = last_b[super_current_b->id];
                route += (this->*algorithm)(temp_entry, exit_b[super_current_b->id]);
            } while (super_current_b != super_target);
            if (super_current_b != super_source)
                do
                {
                    route.insert(0, 1, move_f[super_current_f->id]);
                    Node *temp_exit = exit_f[super_current_f->id];
                    super_current_f = last_f[super_current_f->id];
                    route.insert(0, (this->*algorithm)(entry_f[super_current_f->id], temp_exit));
                } while (super_current_f != super_source);
            delete[] last_f, delete[] exit_f, delete[] entry_f, delete[] move_f,
                delete[] last_b, delete[] exit_b, delete[] entry_b, delete[] move_b;
            return route;
        }

        // track current adjacencies
        for (SuperArc *super_arc : super_current_f->outgoings) // (forwards)
        {
            if (move_f[super_arc->next->id] != 0)
                continue;
            last_f[super_arc->next->id] = super_current_f;
            exit_f[super_arc->next->id] = super_arc->exit;
            entry_f[super_arc->next->id] = super_arc->link->next;
            move_f[super_arc->next->id] = super_arc->link->move;
            open_set_f.push(super_arc->next);
        }
        for (SuperArc *super_arc : super_current_b->incomings) // (backwards)
        {
            if (move_b[super_arc->next->id] != 0)
                continue;
            last_b[super_arc->next->id] = super_current_b;
            entry_b[super_arc->next->id] = super_arc->exit;
            exit_b[super_arc->next->id] = super_arc->link->next;
            move_b[super_arc->next->id] = super_arc->link->move;
            open_set_b.push(super_arc->next);
        }
    }

    // when no path is found
    delete[] last_f, delete[] exit_f, delete[] entry_f, delete[] move_f,
        delete[] last_b, delete[] exit_b, delete[] entry_b, delete[] move_b;
    throw Untraversable(source->position, target->position);
}

Lattice::Route Lattice::super_gbfs(Node *source, Node *target, const SearchMode &sub_search_mode) const
{
    // search meta data
    SuperNode *super_source = source->super, *super_target = target->super,
              **last = new SuperNode *[congraph.size()]();
    Node **exit = new Node *[congraph.size()](),
         **entry = new Node *[congraph.size()]();
    Move *move = new Move[congraph.size()]();
    auto heuristic = [target](Node *n)
    { return manhattan_distance(target->position, n->position); };
    using Pair = std::pair<SuperNode *, size_t>;
    struct PairMinCmp
    {
        bool operator()(const Pair &a, const Pair &b) { return a.second > b.second; }
    };
    std::priority_queue<Pair, std::vector<Pair>, PairMinCmp> open_set;

    // track initial adjacencies
    for (SuperArc *super_arc : super_source->outgoings)
    {
        last[super_arc->next->id] = super_source;
        exit[super_arc->next->id] = super_arc->exit;
        entry[super_arc->next->id] = super_arc->link->next;
        move[super_arc->next->id] = super_arc->link->move;
        open_set.emplace(super_arc->next, heuristic(super_arc->link->next));
    }

    // search
    while (!open_set.empty())
    {
        auto [super_current, key] = open_set.top(); // get next node
        open_set.pop();

        if (super_current == super_target) // goal check
        {
            Algorithm algorithm = get_algorithm(sub_search_mode);
            if (algorithm == nullptr)
                throw InvalidSearchMode(sub_search_mode);
            entry[super_source->id] = source;
            Route route = (this->*algorithm)(entry[super_current->id], target);
            do
            {
                route.insert(0, 1, move[super_current->id]);
                Node *temp_exit = exit[super_current->id];
                super_current = last[super_current->id];
                route.insert(0, (this->*algorithm)(entry[super_current->id], temp_exit));
            } while (super_current != super_source);
            delete[] last, delete[] exit, delete[] entry, delete[] move;
            return route;
        }

        // track current adjacencies
        for (SuperArc *super_arc : super_current->outgoings)
        {
            if (move[super_arc->next->id] != 0)
                continue;
            last[super_arc->next->id] = super_current;
            exit[super_arc->next->id] = super_arc->exit;
            entry[super_arc->next->id] = super_arc->link->next;
            move[super_arc->next->id] = super_arc->link->move;
            open_set.emplace(super_arc->next, heuristic(super_arc->link->next));
        }
    }

    // when no path is found
    delete[] last, delete[] exit, delete[] entry, delete[] move;
    throw Untraversable(source->position, target->position);
}

Lattice::Route Lattice::super_rgbfs(Node *source, Node *target, const SearchMode &sub_search_mode) const
{
    // search meta data
    SuperNode *super_source = source->super, *super_target = target->super,
              **last = new SuperNode *[congraph.size()]();
    Node **exit = new Node *[congraph.size()](),
         **entry = new Node *[congraph.size()]();
    Move *move = new Move[congraph.size()]();
    auto heuristic = [source](Node *n)
    { return manhattan_distance(source->position, n->position); };
    using Pair = std::pair<SuperNode *, size_t>;
    struct PairMinCmp
    {
        bool operator()(const Pair &a, const Pair &b) { return a.second > b.second; }
    };
    std::priority_queue<Pair, std::vector<Pair>, PairMinCmp> open_set;

    // track initial adjacencies
    for (SuperArc *super_arc : super_target->incomings)
    {
        last[super_arc->next->id] = super_target;
        entry[super_arc->next->id] = super_arc->exit;
        exit[super_arc->next->id] = super_arc->link->next;
        move[super_arc->next->id] = super_arc->link->move;
        open_set.emplace(super_arc->next, heuristic(super_arc->link->next));
    }

    // search
    while (!open_set.empty())
    {
        auto [super_current, key] = open_set.top(); // get next node
        open_set.pop();

        if (super_current == super_source) // goal check
        {
            Algorithm algorithm = get_algorithm(sub_search_mode);
            if (algorithm == nullptr)
                throw InvalidSearchMode(sub_search_mode);
            exit[super_target->id] = target;
            Route route = (this->*algorithm)(source, exit[super_current->id]);
            do
            {
                route += move[super_current->id];
                Node *temp_entry = entry[super_current->id];
                super_current = last[super_current->id];
                route += (this->*algorithm)(temp_entry, exit[super_current->id]);
            } while (super_current != super_target);
            delete[] last, delete[] exit, delete[] entry, delete[] move;
            return route;
        }

        // track current adjacencies
        for (SuperArc *super_arc : super_current->incomings)
        {
            if (move[super_arc->next->id] != 0)
                continue;
            last[super_arc->next->id] = super_current;
            entry[super_arc->next->id] = super_arc->exit;
            exit[super_arc->next->id] = super_arc->link->next;
            move[super_arc->next->id] = super_arc->link->move;
            open_set.emplace(super_arc->next, heuristic(super_arc->link->next));
        }
    }

    // when no path is found
    delete[] last, delete[] exit, delete[] entry, delete[] move;
    throw Untraversable(source->position, target->position);
}

Lattice::Route Lattice::super_bdgbfs(Node *source, Node *target, const SearchMode &sub_search_mode) const
{
    // search meta data
    SuperNode *super_source = source->super, *super_target = target->super,
              *super_current_f = super_source, *super_current_b = super_target,
              **last_f = new SuperNode *[congraph.size()](),
              **last_b = new SuperNode *[congraph.size()]();
    Node **exit_f = new Node *[congraph.size()](), **exit_b = new Node *[congraph.size()](),
         **entry_f = new Node *[congraph.size()](), **entry_b = new Node *[congraph.size()](),
         *focus_f = source, *focus_b = target;
    entry_f[super_source->id] = source, exit_b[super_target->id] = target;
    Move *move_f = new Move[congraph.size()](), *move_b = new Move[congraph.size()]();
    auto heuristic_f = [&focus_b](Node *n)
    { return manhattan_distance(focus_b->position, n->position); };
    auto heuristic_b = [&focus_f](Node *n)
    { return manhattan_distance(focus_f->position, n->position); };
    size_t key_f, key_b;
    using Pair = std::pair<SuperNode *, size_t>;
    struct PairMinCmp
    {
        bool operator()(const Pair &a, const Pair &b) { return a.second > b.second; }
    };
    std::priority_queue<Pair, std::vector<Pair>, PairMinCmp> open_set_f, open_set_b;

    // track initial adjacencies
    for (SuperArc *super_arc : super_source->outgoings) // (forwards)
    {
        last_f[super_arc->next->id] = super_source;
        exit_f[super_arc->next->id] = super_arc->exit;
        entry_f[super_arc->next->id] = super_arc->link->next;
        move_f[super_arc->next->id] = super_arc->link->move;
        open_set_f.emplace(super_arc->next, heuristic_f(super_arc->link->next));
    }
    for (SuperArc *super_arc : super_target->incomings) // (backwards)
    {
        last_b[super_arc->next->id] = super_target;
        entry_b[super_arc->next->id] = super_arc->exit;
        exit_b[super_arc->next->id] = super_arc->link->next;
        move_b[super_arc->next->id] = super_arc->link->move;
        open_set_b.emplace(super_arc->next, heuristic_b(super_arc->link->next));
    }

    // search
    while (true)
    {
        if (open_set_f.empty())
            break;

        std::tie(super_current_f, key_f) = open_set_f.top(); // get next super node (forwards)
        open_set_f.pop();

        if (entry_b[super_current_f->id] != nullptr) // goal check (forwards)
        {
            Algorithm algorithm = get_algorithm(sub_search_mode);
            if (algorithm == nullptr)
                throw InvalidSearchMode(sub_search_mode);
            super_current_b = super_current_f;
            Route route = (this->*algorithm)(entry_f[super_current_f->id], exit_b[super_current_f->id]);
            do
            {
                route.insert(0, 1, move_f[super_current_f->id]);
                Node *temp_exit = exit_f[super_current_f->id];
                super_current_f = last_f[super_current_f->id];
                route.insert(0, (this->*algorithm)(entry_f[super_current_f->id], temp_exit));
            } while (super_current_f != super_source);
            if (super_current_b != super_target)
                do
                {
                    route += move_b[super_current_b->id];
                    Node *temp_entry = entry_b[super_current_b->id];
                    super_current_b = last_b[super_current_b->id];
                    route += (this->*algorithm)(temp_entry, exit_b[super_current_b->id]);
                } while (super_current_b != super_target);
            delete[] last_f, delete[] exit_f, delete[] entry_f, delete[] move_f,
                delete[] last_b, delete[] exit_b, delete[] entry_b, delete[] move_b;
            return route;
        }

        if (open_set_b.empty())
            break;
        std::tie(super_current_b, key_b) = open_set_b.top(); // get next super node (backwards)
        open_set_b.pop();

        if (exit_f[super_current_b->id] != nullptr) // goal check (backwards)
        {
            Algorithm algorithm = get_algorithm(sub_search_mode);
            if (algorithm == nullptr)
                throw InvalidSearchMode(sub_search_mode);
            super_current_f = super_current_b;
            Route route = (this->*algorithm)(entry_f[super_current_b->id], exit_b[super_current_b->id]);
            do
            {
                route += move_b[super_current_b->id];
                Node *temp_entry = entry_b[super_current_b->id];
                super_current_b = last_b[super_current_b->id];
                route += (this->*algorithm)(temp_entry, exit_b[super_current_b->id]);
            } while (super_current_b != super_target);
            if (super_current_b != super_source)
                do
                {
                    route.insert(0, 1, move_f[super_current_f->id]);
                    Node *temp_exit = exit_f[super_current_f->id];
                    super_current_f = last_f[super_current_f->id];
                    route.insert(0, (this->*algorithm)(entry_f[super_current_f->id], temp_exit));
                } while (super_current_f != super_source);
            delete[] last_f, delete[] exit_f, delete[] entry_f, delete[] move_f,
                delete[] last_b, delete[] exit_b, delete[] entry_b, delete[] move_b;
            return route;
        }

        // track current adjacencies
        focus_f = entry_f[super_current_f->id];
        for (SuperArc *super_arc : super_current_f->outgoings) // (forwards)
        {
            if (move_f[super_arc->next->id] != 0)
                continue;
            last_f[super_arc->next->id] = super_current_f;
            exit_f[super_arc->next->id] = super_arc->exit;
            entry_f[super_arc->next->id] = super_arc->link->next;
            move_f[super_arc->next->id] = super_arc->link->move;
            open_set_f.emplace(super_arc->next, heuristic_f(super_arc->link->next));
        }
        focus_b = exit_b[super_current_b->id];
        for (SuperArc *super_arc : super_current_b->incomings) // (backwards)
        {
            if (move_b[super_arc->next->id] != 0)
                continue;
            last_b[super_arc->next->id] = super_current_b;
            entry_b[super_arc->next->id] = super_arc->exit;
            exit_b[super_arc->next->id] = super_arc->link->next;
            move_b[super_arc->next->id] = super_arc->link->move;
            open_set_b.emplace(super_arc->next, heuristic_b(super_arc->link->next));
        }
    }

    // when no path is found
    delete[] last_f, delete[] exit_f, delete[] entry_f, delete[] move_f,
        delete[] last_b, delete[] exit_b, delete[] entry_b, delete[] move_b;
    throw Untraversable(source->position, target->position);
}

Lattice::Route Lattice::super_ngbfs(Node *source, Node *target, const SearchMode &sub_search_mode) const
{
    // search meta data
    SuperNode *super_source = source->super, *super_target = target->super,
              **last = new SuperNode *[congraph.size()]();
    Node **exit = new Node *[congraph.size()](),
         **entry = new Node *[congraph.size()]();
    Move *move = new Move[congraph.size()]();
    auto heuristic = [target](Node *n)
    { return manhattan_distance(target->position, n->position); };
    using Pair = std::pair<SuperNode *, size_t>;
    struct PairMaxCmp
    {
        bool operator()(const Pair &a, const Pair &b) { return a.second < b.second; }
    };
    std::priority_queue<Pair, std::vector<Pair>, PairMaxCmp> open_set;

    // track initial adjacencies
    for (SuperArc *super_arc : super_source->outgoings)
    {
        last[super_arc->next->id] = super_source;
        exit[super_arc->next->id] = super_arc->exit;
        entry[super_arc->next->id] = super_arc->link->next;
        move[super_arc->next->id] = super_arc->link->move;
        open_set.emplace(super_arc->next, heuristic(super_arc->link->next));
    }

    // search
    while (!open_set.empty())
    {
        auto [super_current, key] = open_set.top(); // get next node
        open_set.pop();

        if (super_current == super_target) // goal check
        {
            Algorithm algorithm = get_algorithm(sub_search_mode);
            if (algorithm == nullptr)
                throw InvalidSearchMode(sub_search_mode);
            entry[super_source->id] = source;
            Route route = (this->*algorithm)(entry[super_current->id], target);
            do
            {
                route.insert(0, 1, move[super_current->id]);
                Node *temp_exit = exit[super_current->id];
                super_current = last[super_current->id];
                route.insert(0, (this->*algorithm)(entry[super_current->id], temp_exit));
            } while (super_current != super_source);
            delete[] last, delete[] exit, delete[] entry, delete[] move;
            return route;
        }

        // track current adjacencies
        for (SuperArc *super_arc : super_current->outgoings)
        {
            if (move[super_arc->next->id] != 0)
                continue;
            last[super_arc->next->id] = super_current;
            exit[super_arc->next->id] = super_arc->exit;
            entry[super_arc->next->id] = super_arc->link->next;
            move[super_arc->next->id] = super_arc->link->move;
            open_set.emplace(super_arc->next, heuristic(super_arc->link->next));
        }
    }

    // when no path is found
    delete[] last, delete[] exit, delete[] entry, delete[] move;
    throw Untraversable(source->position, target->position);
}

Lattice::Route Lattice::super_rngbfs(Node *source, Node *target, const SearchMode &sub_search_mode) const
{
    // search meta data
    SuperNode *super_source = source->super, *super_target = target->super,
              **last = new SuperNode *[congraph.size()]();
    Node **exit = new Node *[congraph.size()](),
         **entry = new Node *[congraph.size()]();
    Move *move = new Move[congraph.size()]();
    auto heuristic = [source](Node *n)
    { return manhattan_distance(source->position, n->position); };
    using Pair = std::pair<SuperNode *, size_t>;
    struct PairMaxCmp
    {
        bool operator()(const Pair &a, const Pair &b) { return a.second < b.second; }
    };
    std::priority_queue<Pair, std::vector<Pair>, PairMaxCmp> open_set;

    // track initial adjacencies
    for (SuperArc *super_arc : super_target->incomings)
    {
        last[super_arc->next->id] = super_target;
        entry[super_arc->next->id] = super_arc->exit;
        exit[super_arc->next->id] = super_arc->link->next;
        move[super_arc->next->id] = super_arc->link->move;
        open_set.emplace(super_arc->next, heuristic(super_arc->link->next));
    }

    // search
    while (!open_set.empty())
    {
        auto [super_current, key] = open_set.top(); // get next node
        open_set.pop();

        if (super_current == super_source) // goal check
        {
            Algorithm algorithm = get_algorithm(sub_search_mode);
            if (algorithm == nullptr)
                throw InvalidSearchMode(sub_search_mode);
            exit[super_target->id] = target;
            Route route = (this->*algorithm)(source, exit[super_current->id]);
            do
            {
                route += move[super_current->id];
                Node *temp_entry = entry[super_current->id];
                super_current = last[super_current->id];
                route += (this->*algorithm)(temp_entry, exit[super_current->id]);
            } while (super_current != super_target);
            delete[] last, delete[] exit, delete[] entry, delete[] move;
            return route;
        }

        // track current adjacencies
        for (SuperArc *super_arc : super_current->incomings)
        {
            if (move[super_arc->next->id] != 0)
                continue;
            last[super_arc->next->id] = super_current;
            entry[super_arc->next->id] = super_arc->exit;
            exit[super_arc->next->id] = super_arc->link->next;
            move[super_arc->next->id] = super_arc->link->move;
            open_set.emplace(super_arc->next, heuristic(super_arc->link->next));
        }
    }

    // when no path is found
    delete[] last, delete[] exit, delete[] entry, delete[] move;
    throw Untraversable(source->position, target->position);
}

Lattice::Route Lattice::super_bdngbfs(Node *source, Node *target, const SearchMode &sub_search_mode) const
{
    // search meta data
    SuperNode *super_source = source->super, *super_target = target->super,
              *super_current_f = super_source, *super_current_b = super_target,
              **last_f = new SuperNode *[congraph.size()](),
              **last_b = new SuperNode *[congraph.size()]();
    Node **exit_f = new Node *[congraph.size()](), **exit_b = new Node *[congraph.size()](),
         **entry_f = new Node *[congraph.size()](), **entry_b = new Node *[congraph.size()](),
         *focus_f = source, *focus_b = target;
    entry_f[super_source->id] = source, exit_b[super_target->id] = target;
    Move *move_f = new Move[congraph.size()](), *move_b = new Move[congraph.size()]();
    auto heuristic_f = [&focus_b](Node *n)
    { return manhattan_distance(focus_b->position, n->position); };
    auto heuristic_b = [&focus_f](Node *n)
    { return manhattan_distance(focus_f->position, n->position); };
    size_t key_f, key_b;
    using Pair = std::pair<SuperNode *, size_t>;
    struct PairMaxCmp
    {
        bool operator()(const Pair &a, const Pair &b) { return a.second < b.second; }
    };
    std::priority_queue<Pair, std::vector<Pair>, PairMaxCmp> open_set_f, open_set_b;

    // track initial adjacencies
    for (SuperArc *super_arc : super_source->outgoings) // (forwards)
    {
        last_f[super_arc->next->id] = super_source;
        exit_f[super_arc->next->id] = super_arc->exit;
        entry_f[super_arc->next->id] = super_arc->link->next;
        move_f[super_arc->next->id] = super_arc->link->move;
        open_set_f.emplace(super_arc->next, heuristic_f(super_arc->link->next));
    }
    for (SuperArc *super_arc : super_target->incomings) // (backwards)
    {
        last_b[super_arc->next->id] = super_target;
        entry_b[super_arc->next->id] = super_arc->exit;
        exit_b[super_arc->next->id] = super_arc->link->next;
        move_b[super_arc->next->id] = super_arc->link->move;
        open_set_b.emplace(super_arc->next, heuristic_b(super_arc->link->next));
    }

    // search
    while (true)
    {
        if (open_set_f.empty())
            break;

        std::tie(super_current_f, key_f) = open_set_f.top(); // get next super node (forwards)
        open_set_f.pop();

        if (entry_b[super_current_f->id] != nullptr) // goal check (forwards)
        {
            Algorithm algorithm = get_algorithm(sub_search_mode);
            if (algorithm == nullptr)
                throw InvalidSearchMode(sub_search_mode);
            super_current_b = super_current_f;
            Route route = (this->*algorithm)(entry_f[super_current_f->id], exit_b[super_current_f->id]);
            do
            {
                route.insert(0, 1, move_f[super_current_f->id]);
                Node *temp_exit = exit_f[super_current_f->id];
                super_current_f = last_f[super_current_f->id];
                route.insert(0, (this->*algorithm)(entry_f[super_current_f->id], temp_exit));
            } while (super_current_f != super_source);
            if (super_current_b != super_target)
                do
                {
                    route += move_b[super_current_b->id];
                    Node *temp_entry = entry_b[super_current_b->id];
                    super_current_b = last_b[super_current_b->id];
                    route += (this->*algorithm)(temp_entry, exit_b[super_current_b->id]);
                } while (super_current_b != super_target);
            delete[] last_f, delete[] exit_f, delete[] entry_f, delete[] move_f,
                delete[] last_b, delete[] exit_b, delete[] entry_b, delete[] move_b;
            return route;
        }

        if (open_set_b.empty())
            break;
        std::tie(super_current_b, key_b) = open_set_b.top(); // get next super node (backwards)
        open_set_b.pop();

        if (exit_f[super_current_b->id] != nullptr) // goal check (backwards)
        {
            Algorithm algorithm = get_algorithm(sub_search_mode);
            if (algorithm == nullptr)
                throw InvalidSearchMode(sub_search_mode);
            super_current_f = super_current_b;
            Route route = (this->*algorithm)(entry_f[super_current_b->id], exit_b[super_current_b->id]);
            do
            {
                route += move_b[super_current_b->id];
                Node *temp_entry = entry_b[super_current_b->id];
                super_current_b = last_b[super_current_b->id];
                route += (this->*algorithm)(temp_entry, exit_b[super_current_b->id]);
            } while (super_current_b != super_target);
            if (super_current_b != super_source)
                do
                {
                    route.insert(0, 1, move_f[super_current_f->id]);
                    Node *temp_exit = exit_f[super_current_f->id];
                    super_current_f = last_f[super_current_f->id];
                    route.insert(0, (this->*algorithm)(entry_f[super_current_f->id], temp_exit));
                } while (super_current_f != super_source);
            delete[] last_f, delete[] exit_f, delete[] entry_f, delete[] move_f,
                delete[] last_b, delete[] exit_b, delete[] entry_b, delete[] move_b;
            return route;
        }

        // track current adjacencies
        focus_f = entry_f[super_current_f->id];
        for (SuperArc *super_arc : super_current_f->outgoings) // (forwards)
        {
            if (move_f[super_arc->next->id] != 0)
                continue;
            last_f[super_arc->next->id] = super_current_f;
            exit_f[super_arc->next->id] = super_arc->exit;
            entry_f[super_arc->next->id] = super_arc->link->next;
            move_f[super_arc->next->id] = super_arc->link->move;
            open_set_f.emplace(super_arc->next, heuristic_f(super_arc->link->next));
        }
        focus_b = exit_b[super_current_b->id];
        for (SuperArc *super_arc : super_current_b->incomings) // (backwards)
        {
            if (move_b[super_arc->next->id] != 0)
                continue;
            last_b[super_arc->next->id] = super_current_b;
            entry_b[super_arc->next->id] = super_arc->exit;
            exit_b[super_arc->next->id] = super_arc->link->next;
            move_b[super_arc->next->id] = super_arc->link->move;
            open_set_b.emplace(super_arc->next, heuristic_b(super_arc->link->next));
        }
    }

    // when no path is found
    delete[] last_f, delete[] exit_f, delete[] entry_f, delete[] move_f,
        delete[] last_b, delete[] exit_b, delete[] entry_b, delete[] move_b;
    throw Untraversable(source->position, target->position);
}

Lattice::Route Lattice::super_astar(Node *source, Node *target, const SearchMode &sub_search_mode) const
{
    // search meta data
    SuperNode *super_source = source->super, *super_target = target->super,
              **last = new SuperNode *[congraph.size()]();
    Node **exit = new Node *[congraph.size()](),
         **entry = new Node *[congraph.size()]();
    Move *move = new Move[congraph.size()]();
    auto heuristic = [source, target](Node *n)
    { return manhattan_distance(source->position, n->position) +
             manhattan_distance(target->position, n->position); };
    using Pair = std::pair<SuperNode *, size_t>;
    struct PairMinCmp
    {
        bool operator()(const Pair &a, const Pair &b) { return a.second > b.second; }
    };
    std::priority_queue<Pair, std::vector<Pair>, PairMinCmp> open_set;

    // track initial adjacencies
    for (SuperArc *super_arc : super_source->outgoings)
    {
        last[super_arc->next->id] = super_source;
        exit[super_arc->next->id] = super_arc->exit;
        entry[super_arc->next->id] = super_arc->link->next;
        move[super_arc->next->id] = super_arc->link->move;
        open_set.emplace(super_arc->next, heuristic(super_arc->link->next));
    }

    // search
    while (!open_set.empty())
    {
        auto [super_current, key] = open_set.top(); // get next node
        open_set.pop();

        if (super_current == super_target) // goal check
        {
            Algorithm algorithm = get_algorithm(sub_search_mode);
            if (algorithm == nullptr)
                throw InvalidSearchMode(sub_search_mode);
            entry[super_source->id] = source;
            Route route = (this->*algorithm)(entry[super_current->id], target);
            do
            {
                route.insert(0, 1, move[super_current->id]);
                Node *temp_exit = exit[super_current->id];
                super_current = last[super_current->id];
                route.insert(0, (this->*algorithm)(entry[super_current->id], temp_exit));
            } while (super_current != super_source);
            delete[] last, delete[] exit, delete[] entry, delete[] move;
            return route;
        }

        // track current adjacencies
        for (SuperArc *super_arc : super_current->outgoings)
        {
            if (move[super_arc->next->id] != 0)
                continue;
            last[super_arc->next->id] = super_current;
            exit[super_arc->next->id] = super_arc->exit;
            entry[super_arc->next->id] = super_arc->link->next;
            move[super_arc->next->id] = super_arc->link->move;
            open_set.emplace(super_arc->next, heuristic(super_arc->link->next));
        }
    }

    // when no path is found
    delete[] last, delete[] exit, delete[] entry, delete[] move;
    throw Untraversable(source->position, target->position);
}

Lattice::Route Lattice::super_rastar(Node *source, Node *target, const SearchMode &sub_search_mode) const
{
    // search meta data
    SuperNode *super_source = source->super, *super_target = target->super,
              **last = new SuperNode *[congraph.size()]();
    Node **exit = new Node *[congraph.size()](),
         **entry = new Node *[congraph.size()]();
    Move *move = new Move[congraph.size()]();
    auto heuristic = [target, source](Node *n)
    { return manhattan_distance(target->position, n->position) +
             manhattan_distance(source->position, n->position); };
    using Pair = std::pair<SuperNode *, size_t>;
    struct PairMinCmp
    {
        bool operator()(const Pair &a, const Pair &b) { return a.second > b.second; }
    };
    std::priority_queue<Pair, std::vector<Pair>, PairMinCmp> open_set;

    // track initial adjacencies
    for (SuperArc *super_arc : super_target->incomings)
    {
        last[super_arc->next->id] = super_target;
        entry[super_arc->next->id] = super_arc->exit;
        exit[super_arc->next->id] = super_arc->link->next;
        move[super_arc->next->id] = super_arc->link->move;
        open_set.emplace(super_arc->next, heuristic(super_arc->link->next));
    }

    // search
    while (!open_set.empty())
    {
        auto [super_current, key] = open_set.top(); // get next node
        open_set.pop();

        if (super_current == super_source) // goal check
        {
            Algorithm algorithm = get_algorithm(sub_search_mode);
            if (algorithm == nullptr)
                throw InvalidSearchMode(sub_search_mode);
            exit[super_target->id] = target;
            Route route = (this->*algorithm)(source, exit[super_current->id]);
            do
            {
                route += move[super_current->id];
                Node *temp_entry = entry[super_current->id];
                super_current = last[super_current->id];
                route += (this->*algorithm)(temp_entry, exit[super_current->id]);
            } while (super_current != super_target);
            delete[] last, delete[] exit, delete[] entry, delete[] move;
            return route;
        }

        // track current adjacencies
        for (SuperArc *super_arc : super_current->incomings)
        {
            if (move[super_arc->next->id] != 0)
                continue;
            last[super_arc->next->id] = super_current;
            entry[super_arc->next->id] = super_arc->exit;
            exit[super_arc->next->id] = super_arc->link->next;
            move[super_arc->next->id] = super_arc->link->move;
            open_set.emplace(super_arc->next, heuristic(super_arc->link->next));
        }
    }

    // when no path is found
    delete[] last, delete[] exit, delete[] entry, delete[] move;
    throw Untraversable(source->position, target->position);
}

Lattice::Route Lattice::super_bdastar(Node *source, Node *target, const SearchMode &sub_search_mode) const
{
    // search meta data
    SuperNode *super_source = source->super, *super_target = target->super,
              *super_current_f = super_source, *super_current_b = super_target,
              **last_f = new SuperNode *[congraph.size()](),
              **last_b = new SuperNode *[congraph.size()]();
    Node **exit_f = new Node *[congraph.size()](), **exit_b = new Node *[congraph.size()](),
         **entry_f = new Node *[congraph.size()](), **entry_b = new Node *[congraph.size()](),
         *focus_f = source, *focus_b = target;
    entry_f[super_source->id] = source, exit_b[super_target->id] = target;
    Move *move_f = new Move[congraph.size()](), *move_b = new Move[congraph.size()]();
    auto heuristic_f = [source, &focus_b](Node *n)
    { return manhattan_distance(source->position, n->position) +
             manhattan_distance(focus_b->position, n->position); };
    auto heuristic_b = [target, &focus_f](Node *n)
    { return manhattan_distance(target->position, n->position) +
             manhattan_distance(focus_f->position, n->position); };
    size_t key_f, key_b;
    using Pair = std::pair<SuperNode *, size_t>;
    struct PairMinCmp
    {
        bool operator()(const Pair &a, const Pair &b) { return a.second > b.second; }
    };
    std::priority_queue<Pair, std::vector<Pair>, PairMinCmp> open_set_f, open_set_b;

    // track initial adjacencies
    for (SuperArc *super_arc : super_source->outgoings) // (forwards)
    {
        last_f[super_arc->next->id] = super_source;
        exit_f[super_arc->next->id] = super_arc->exit;
        entry_f[super_arc->next->id] = super_arc->link->next;
        move_f[super_arc->next->id] = super_arc->link->move;
        open_set_f.emplace(super_arc->next, heuristic_f(super_arc->link->next));
    }
    for (SuperArc *super_arc : super_target->incomings) // (backwards)
    {
        last_b[super_arc->next->id] = super_target;
        entry_b[super_arc->next->id] = super_arc->exit;
        exit_b[super_arc->next->id] = super_arc->link->next;
        move_b[super_arc->next->id] = super_arc->link->move;
        open_set_b.emplace(super_arc->next, heuristic_b(super_arc->link->next));
    }

    // search
    while (true)
    {
        if (open_set_f.empty())
            break;

        std::tie(super_current_f, key_f) = open_set_f.top(); // get next super node (forwards)
        open_set_f.pop();

        if (entry_b[super_current_f->id] != nullptr) // goal check (forwards)
        {
            Algorithm algorithm = get_algorithm(sub_search_mode);
            if (algorithm == nullptr)
                throw InvalidSearchMode(sub_search_mode);
            super_current_b = super_current_f;
            Route route = (this->*algorithm)(entry_f[super_current_f->id], exit_b[super_current_f->id]);
            do
            {
                route.insert(0, 1, move_f[super_current_f->id]);
                Node *temp_exit = exit_f[super_current_f->id];
                super_current_f = last_f[super_current_f->id];
                route.insert(0, (this->*algorithm)(entry_f[super_current_f->id], temp_exit));
            } while (super_current_f != super_source);
            if (super_current_b != super_target)
                do
                {
                    route += move_b[super_current_b->id];
                    Node *temp_entry = entry_b[super_current_b->id];
                    super_current_b = last_b[super_current_b->id];
                    route += (this->*algorithm)(temp_entry, exit_b[super_current_b->id]);
                } while (super_current_b != super_target);
            delete[] last_f, delete[] exit_f, delete[] entry_f, delete[] move_f,
                delete[] last_b, delete[] exit_b, delete[] entry_b, delete[] move_b;
            return route;
        }

        if (open_set_b.empty())
            break;
        std::tie(super_current_b, key_b) = open_set_b.top(); // get next super node (backwards)
        open_set_b.pop();

        if (exit_f[super_current_b->id] != nullptr) // goal check (backwards)
        {
            Algorithm algorithm = get_algorithm(sub_search_mode);
            if (algorithm == nullptr)
                throw InvalidSearchMode(sub_search_mode);
            super_current_f = super_current_b;
            Route route = (this->*algorithm)(entry_f[super_current_b->id], exit_b[super_current_b->id]);
            do
            {
                route += move_b[super_current_b->id];
                Node *temp_entry = entry_b[super_current_b->id];
                super_current_b = last_b[super_current_b->id];
                route += (this->*algorithm)(temp_entry, exit_b[super_current_b->id]);
            } while (super_current_b != super_target);
            if (super_current_b != super_source)
                do
                {
                    route.insert(0, 1, move_f[super_current_f->id]);
                    Node *temp_exit = exit_f[super_current_f->id];
                    super_current_f = last_f[super_current_f->id];
                    route.insert(0, (this->*algorithm)(entry_f[super_current_f->id], temp_exit));
                } while (super_current_f != super_source);
            delete[] last_f, delete[] exit_f, delete[] entry_f, delete[] move_f,
                delete[] last_b, delete[] exit_b, delete[] entry_b, delete[] move_b;
            return route;
        }

        // track current adjacencies
        focus_f = entry_f[super_current_f->id];
        for (SuperArc *super_arc : super_current_f->outgoings) // (forwards)
        {
            if (move_f[super_arc->next->id] != 0)
                continue;
            last_f[super_arc->next->id] = super_current_f;
            exit_f[super_arc->next->id] = super_arc->exit;
            entry_f[super_arc->next->id] = super_arc->link->next;
            move_f[super_arc->next->id] = super_arc->link->move;
            open_set_f.emplace(super_arc->next, heuristic_f(super_arc->link->next));
        }
        focus_b = exit_b[super_current_b->id];
        for (SuperArc *super_arc : super_current_b->incomings) // (backwards)
        {
            if (move_b[super_arc->next->id] != 0)
                continue;
            last_b[super_arc->next->id] = super_current_b;
            entry_b[super_arc->next->id] = super_arc->exit;
            exit_b[super_arc->next->id] = super_arc->link->next;
            move_b[super_arc->next->id] = super_arc->link->move;
            open_set_b.emplace(super_arc->next, heuristic_b(super_arc->link->next));
        }
    }

    // when no path is found
    delete[] last_f, delete[] exit_f, delete[] entry_f, delete[] move_f,
        delete[] last_b, delete[] exit_b, delete[] entry_b, delete[] move_b;
    throw Untraversable(source->position, target->position);
}

Lattice::Route Lattice::super_nastar(Node *source, Node *target, const SearchMode &sub_search_mode) const
{
    // search meta data
    SuperNode *super_source = source->super, *super_target = target->super,
              **last = new SuperNode *[congraph.size()]();
    Node **exit = new Node *[congraph.size()](),
         **entry = new Node *[congraph.size()]();
    Move *move = new Move[congraph.size()]();
    auto heuristic = [source, target](Node *n)
    { return manhattan_distance(source->position, n->position) +
             manhattan_distance(target->position, n->position); };
    using Pair = std::pair<SuperNode *, size_t>;
    struct PairMaxCmp
    {
        bool operator()(const Pair &a, const Pair &b) { return a.second < b.second; }
    };
    std::priority_queue<Pair, std::vector<Pair>, PairMaxCmp> open_set;

    // track initial adjacencies
    for (SuperArc *super_arc : super_source->outgoings)
    {
        last[super_arc->next->id] = super_source;
        exit[super_arc->next->id] = super_arc->exit;
        entry[super_arc->next->id] = super_arc->link->next;
        move[super_arc->next->id] = super_arc->link->move;
        open_set.emplace(super_arc->next, heuristic(super_arc->link->next));
    }

    // search
    while (!open_set.empty())
    {
        auto [super_current, key] = open_set.top(); // get next node
        open_set.pop();

        if (super_current == super_target) // goal check
        {
            Algorithm algorithm = get_algorithm(sub_search_mode);
            if (algorithm == nullptr)
                throw InvalidSearchMode(sub_search_mode);
            entry[super_source->id] = source;
            Route route = (this->*algorithm)(entry[super_current->id], target);
            do
            {
                route.insert(0, 1, move[super_current->id]);
                Node *temp_exit = exit[super_current->id];
                super_current = last[super_current->id];
                route.insert(0, (this->*algorithm)(entry[super_current->id], temp_exit));
            } while (super_current != super_source);
            delete[] last, delete[] exit, delete[] entry, delete[] move;
            return route;
        }

        // track current adjacencies
        for (SuperArc *super_arc : super_current->outgoings)
        {
            if (move[super_arc->next->id] != 0)
                continue;
            last[super_arc->next->id] = super_current;
            exit[super_arc->next->id] = super_arc->exit;
            entry[super_arc->next->id] = super_arc->link->next;
            move[super_arc->next->id] = super_arc->link->move;
            open_set.emplace(super_arc->next, heuristic(super_arc->link->next));
        }
    }

    // when no path is found
    delete[] last, delete[] exit, delete[] entry, delete[] move;
    throw Untraversable(source->position, target->position);
}

Lattice::Route Lattice::super_rnastar(Node *source, Node *target, const SearchMode &sub_search_mode) const
{
    // search meta data
    SuperNode *super_source = source->super, *super_target = target->super,
              **last = new SuperNode *[congraph.size()]();
    Node **exit = new Node *[congraph.size()](),
         **entry = new Node *[congraph.size()]();
    Move *move = new Move[congraph.size()]();
    auto heuristic = [target, source](Node *n)
    { return manhattan_distance(target->position, n->position) +
             manhattan_distance(source->position, n->position); };
    using Pair = std::pair<SuperNode *, size_t>;
    struct PairMaxCmp
    {
        bool operator()(const Pair &a, const Pair &b) { return a.second < b.second; }
    };
    std::priority_queue<Pair, std::vector<Pair>, PairMaxCmp> open_set;

    // track initial adjacencies
    for (SuperArc *super_arc : super_target->incomings)
    {
        last[super_arc->next->id] = super_target;
        entry[super_arc->next->id] = super_arc->exit;
        exit[super_arc->next->id] = super_arc->link->next;
        move[super_arc->next->id] = super_arc->link->move;
        open_set.emplace(super_arc->next, heuristic(super_arc->link->next));
    }

    // search
    while (!open_set.empty())
    {
        auto [super_current, key] = open_set.top(); // get next node
        open_set.pop();

        if (super_current == super_source) // goal check
        {
            Algorithm algorithm = get_algorithm(sub_search_mode);
            if (algorithm == nullptr)
                throw InvalidSearchMode(sub_search_mode);
            exit[super_target->id] = target;
            Route route = (this->*algorithm)(source, exit[super_current->id]);
            do
            {
                route += move[super_current->id];
                Node *temp_entry = entry[super_current->id];
                super_current = last[super_current->id];
                route += (this->*algorithm)(temp_entry, exit[super_current->id]);
            } while (super_current != super_target);
            delete[] last, delete[] exit, delete[] entry, delete[] move;
            return route;
        }

        // track current adjacencies
        for (SuperArc *super_arc : super_current->incomings)
        {
            if (move[super_arc->next->id] != 0)
                continue;
            last[super_arc->next->id] = super_current;
            entry[super_arc->next->id] = super_arc->exit;
            exit[super_arc->next->id] = super_arc->link->next;
            move[super_arc->next->id] = super_arc->link->move;
            open_set.emplace(super_arc->next, heuristic(super_arc->link->next));
        }
    }

    // when no path is found
    delete[] last, delete[] exit, delete[] entry, delete[] move;
    throw Untraversable(source->position, target->position);
}

Lattice::Route Lattice::super_bdnastar(Node *source, Node *target, const SearchMode &sub_search_mode) const
{
    // search meta data
    SuperNode *super_source = source->super, *super_target = target->super,
              *super_current_f = super_source, *super_current_b = super_target,
              **last_f = new SuperNode *[congraph.size()](),
              **last_b = new SuperNode *[congraph.size()]();
    Node **exit_f = new Node *[congraph.size()](), **exit_b = new Node *[congraph.size()](),
         **entry_f = new Node *[congraph.size()](), **entry_b = new Node *[congraph.size()](),
         *focus_f = source, *focus_b = target;
    entry_f[super_source->id] = source, exit_b[super_target->id] = target;
    Move *move_f = new Move[congraph.size()](), *move_b = new Move[congraph.size()]();
    auto heuristic_f = [source, &focus_b](Node *n)
    { return manhattan_distance(source->position, n->position) +
             manhattan_distance(focus_b->position, n->position); };
    auto heuristic_b = [target, &focus_f](Node *n)
    { return manhattan_distance(target->position, n->position) +
             manhattan_distance(focus_f->position, n->position); };
    size_t key_f, key_b;
    using Pair = std::pair<SuperNode *, size_t>;
    struct PairMaxCmp
    {
        bool operator()(const Pair &a, const Pair &b) { return a.second < b.second; }
    };
    std::priority_queue<Pair, std::vector<Pair>, PairMaxCmp> open_set_f, open_set_b;

    // track initial adjacencies
    for (SuperArc *super_arc : super_source->outgoings) // (forwards)
    {
        last_f[super_arc->next->id] = super_source;
        exit_f[super_arc->next->id] = super_arc->exit;
        entry_f[super_arc->next->id] = super_arc->link->next;
        move_f[super_arc->next->id] = super_arc->link->move;
        open_set_f.emplace(super_arc->next, heuristic_f(super_arc->link->next));
    }
    for (SuperArc *super_arc : super_target->incomings) // (backwards)
    {
        last_b[super_arc->next->id] = super_target;
        entry_b[super_arc->next->id] = super_arc->exit;
        exit_b[super_arc->next->id] = super_arc->link->next;
        move_b[super_arc->next->id] = super_arc->link->move;
        open_set_b.emplace(super_arc->next, heuristic_b(super_arc->link->next));
    }

    // search
    while (true)
    {
        if (open_set_f.empty())
            break;

        std::tie(super_current_f, key_f) = open_set_f.top(); // get next super node (forwards)
        open_set_f.pop();

        if (entry_b[super_current_f->id] != nullptr) // goal check (forwards)
        {
            Algorithm algorithm = get_algorithm(sub_search_mode);
            if (algorithm == nullptr)
                throw InvalidSearchMode(sub_search_mode);
            super_current_b = super_current_f;
            Route route = (this->*algorithm)(entry_f[super_current_f->id], exit_b[super_current_f->id]);
            do
            {
                route.insert(0, 1, move_f[super_current_f->id]);
                Node *temp_exit = exit_f[super_current_f->id];
                super_current_f = last_f[super_current_f->id];
                route.insert(0, (this->*algorithm)(entry_f[super_current_f->id], temp_exit));
            } while (super_current_f != super_source);
            if (super_current_b != super_target)
                do
                {
                    route += move_b[super_current_b->id];
                    Node *temp_entry = entry_b[super_current_b->id];
                    super_current_b = last_b[super_current_b->id];
                    route += (this->*algorithm)(temp_entry, exit_b[super_current_b->id]);
                } while (super_current_b != super_target);
            delete[] last_f, delete[] exit_f, delete[] entry_f, delete[] move_f,
                delete[] last_b, delete[] exit_b, delete[] entry_b, delete[] move_b;
            return route;
        }

        if (open_set_b.empty())
            break;
        std::tie(super_current_b, key_b) = open_set_b.top(); // get next super node (backwards)
        open_set_b.pop();

        if (exit_f[super_current_b->id] != nullptr) // goal check (backwards)
        {
            Algorithm algorithm = get_algorithm(sub_search_mode);
            if (algorithm == nullptr)
                throw InvalidSearchMode(sub_search_mode);
            super_current_f = super_current_b;
            Route route = (this->*algorithm)(entry_f[super_current_b->id], exit_b[super_current_b->id]);
            do
            {
                route += move_b[super_current_b->id];
                Node *temp_entry = entry_b[super_current_b->id];
                super_current_b = last_b[super_current_b->id];
                route += (this->*algorithm)(temp_entry, exit_b[super_current_b->id]);
            } while (super_current_b != super_target);
            if (super_current_b != super_source)
                do
                {
                    route.insert(0, 1, move_f[super_current_f->id]);
                    Node *temp_exit = exit_f[super_current_f->id];
                    super_current_f = last_f[super_current_f->id];
                    route.insert(0, (this->*algorithm)(entry_f[super_current_f->id], temp_exit));
                } while (super_current_f != super_source);
            delete[] last_f, delete[] exit_f, delete[] entry_f, delete[] move_f,
                delete[] last_b, delete[] exit_b, delete[] entry_b, delete[] move_b;
            return route;
        }

        // track current adjacencies
        focus_f = entry_f[super_current_f->id];
        for (SuperArc *super_arc : super_current_f->outgoings) // (forwards)
        {
            if (move_f[super_arc->next->id] != 0)
                continue;
            last_f[super_arc->next->id] = super_current_f;
            exit_f[super_arc->next->id] = super_arc->exit;
            entry_f[super_arc->next->id] = super_arc->link->next;
            move_f[super_arc->next->id] = super_arc->link->move;
            open_set_f.emplace(super_arc->next, heuristic_f(super_arc->link->next));
        }
        focus_b = exit_b[super_current_b->id];
        for (SuperArc *super_arc : super_current_b->incomings) // (backwards)
        {
            if (move_b[super_arc->next->id] != 0)
                continue;
            last_b[super_arc->next->id] = super_current_b;
            entry_b[super_arc->next->id] = super_arc->exit;
            exit_b[super_arc->next->id] = super_arc->link->next;
            move_b[super_arc->next->id] = super_arc->link->move;
            open_set_b.emplace(super_arc->next, heuristic_b(super_arc->link->next));
        }
    }

    // when no path is found
    delete[] last_f, delete[] exit_f, delete[] entry_f, delete[] move_f,
        delete[] last_b, delete[] exit_b, delete[] entry_b, delete[] move_b;
    throw Untraversable(source->position, target->position);
}

bool Lattice::verify(const SearchMode &search_mode) const
{
    Algorithm algorithm = get_algorithm(search_mode);
    if (algorithm == nullptr)
        throw InvalidSearchMode(search_mode);
    for (auto &[sp, sn] : graph)
    {
        for (auto &[tp, tn] : graph)
        {
            Route route;
            try
            {
                route = (this->*algorithm)(sn, tn);
            }
            catch (const std::exception &e)
            {
                continue;
            }
            if (travel(sp, route) != tp)
            {
                LOG << sp << tp;
                return false;
            }
        }
    }
    return true;
}

bool Lattice::super_verify(const SearchMode &super_search_mode,
                           const SearchMode &sub_search_mode) const
{
    SuperAlgorithm super_algorithm = get_super_algorithm(super_search_mode);
    if (super_algorithm == nullptr)
        throw InvalidSearchMode(super_search_mode);

    Algorithm algorithm = get_algorithm(sub_search_mode);
    if (algorithm == nullptr)
        throw InvalidSearchMode(sub_search_mode);

    for (SuperNode *super1 : congraph)
    {
        if (super1->internals.size() == 2) // ignore windows in bastion
            continue;
        for (SuperNode *super2 : congraph)
        {
            if (super2->internals.size() == 2) // ignore windows in bastion
                continue;
            Node *sn = super1->internals.front(),
                 *tn = super2->internals.back();
            Route route;
            try
            {
                route = (this->*super_algorithm)(sn, tn, sub_search_mode);
            }
            catch (const std::exception &e)
            {
                continue;
            }
            if (travel(sn->position, route) != tn->position)
            {
                LOG << sn->position << tn->position;
                return false;
            }
        }
    }
    return true;
}
