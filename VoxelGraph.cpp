#include "VoxelGraph.hpp"

#define PRINT std::cout <<
#define NL '\n'
#define HELP PRINT "HELP~VoxelGraph\n"

#define VOID 7
#define SOLID 4
#define NEW_NODE 2
#define TWO_WAY_NODE 1
#define ONE_WAY_NODE 0

#define UPDATE >>= 1

#define GO_WEST -1
#define GO_NORTH -x_limit
#define GO_DOWN -map_area
#define FALL -= map_area

#define INVERSE -map_area

#define HAS_NEIGHBOR < 4

VoxelGraph::VoxelGraph(std::istream &stream) // TODO bad parse handling
{
    stream >> x_limit >> y_limit >> z_limit; // initialize world upper bounds

    uint8_t schematic[(map_area = x_limit * y_limit)]; // initialize schematic

    node_map = new Node *[map_volume = map_area * z_limit]; // initialize graph
    std::memset(node_map, 0, map_volume * sizeof(Node *));

    // populate schematic
    std::string row;
    size_t schematic_index = 0;
    for (int y = 0; y < y_limit; ++y)
    {
        stream >> row;
        for (char character : row)
            for (uint8_t B : bit_mask)
                schematic[schematic_index++] = (hex_to_dec(character) & B) ? SOLID : VOID;
    }

    // helper lambda functions
    auto landing = [this](size_t index)
    {
        while (node_map[index] == nullptr)
            index FALL;
        return index;
    };
    auto directed_link = [this](size_t from, size_t to, char direction)
    {
        node_map[from]->next_moves.emplace_back(node_map[to], direction);
        node_map[to]->next_moves_T.emplace_back(node_map[from], direction);
    };

    // populate graph
    size_t current_index = map_area;
    for (int z = 1; z < z_limit; ++z)
    {
        schematic_index = 0;
        for (int y = 0; y < y_limit; ++y)
        {
            stream >> row;
            int x = 0;
            for (char character : row)
            {
                for (uint8_t i = 0; i < 4; ++i, ++current_index, ++schematic_index, ++x)
                {
                    if (hex_to_dec(character) & bit_mask[i])
                        schematic[schematic_index] = SOLID;
                    else // current voxel is not SOLID
                    {
                        if (schematic[schematic_index] == VOID)
                            continue;
                        schematic[schematic_index] UPDATE;
                        if (schematic[schematic_index] == NEW_NODE)
                        {
                            node_map[current_index] = new Node(Coordinate(x, y, z));
                            size_t node_index = current_index;
                            if (x != 0 && schematic[schematic_index GO_WEST] HAS_NEIGHBOR)
                            {
                                size_t node_neighbor_index = landing(current_index GO_WEST);
                                directed_link(node_index, node_neighbor_index, 'w');
                                if (schematic[schematic_index GO_WEST] != ONE_WAY_NODE)
                                    directed_link(node_neighbor_index, node_index, 'e');
                            }
                            if (y != 0 && schematic[schematic_index GO_NORTH] HAS_NEIGHBOR)
                            {
                                size_t node_neighbor_index = landing(current_index GO_NORTH);
                                directed_link(node_index, node_neighbor_index, 'n');
                                if (schematic[schematic_index GO_NORTH] != ONE_WAY_NODE)
                                    directed_link(node_neighbor_index, node_index, 's');
                            }
                        }
                        if (schematic[schematic_index] == TWO_WAY_NODE)
                        {
                            size_t node_index = current_index GO_DOWN;
                            if (x != 0 && schematic[schematic_index GO_WEST] == NEW_NODE)
                            {
                                size_t node_neighbor_index = current_index GO_WEST;
                                directed_link(node_index, node_neighbor_index, 'w');
                                directed_link(node_neighbor_index, node_index, 'e');
                            }
                            if (y != 0 && schematic[schematic_index GO_NORTH] == NEW_NODE)
                            {
                                size_t node_neighbor_index = current_index GO_NORTH;
                                directed_link(node_index, node_neighbor_index, 'n');
                                directed_link(node_neighbor_index, node_index, 's');
                            }
                        }
                        if (schematic[schematic_index] == ONE_WAY_NODE)
                        {
                            size_t node_index = landing(current_index GO_DOWN GO_DOWN);
                            if (x != 0 && schematic[schematic_index GO_WEST] == NEW_NODE)
                            {
                                size_t node_neighbor_index = current_index GO_WEST;
                                directed_link(node_neighbor_index, node_index, 'e');
                            }
                            if (y != 0 && schematic[schematic_index GO_NORTH] == NEW_NODE)
                            {
                                size_t node_neighbor_index = current_index GO_NORTH;
                                directed_link(node_neighbor_index, node_index, 's');
                            }
                        }
                    }
                }
            }
        }
    }
}

void VoxelGraph::DFS1(Node *current_node, std::stack<Node *> &stack) const
{
    current_node->visit_id = current_visit_id;
    for (Node::Move &next_move : current_node->next_moves)
        if (!visited(next_move.node))
            DFS1(next_move.node, stack);
    stack.push(current_node);
}

void VoxelGraph::DFS2(Node *current_node, SuperNode *super_node) const
{
    current_node->visit_id = current_visit_id;
    current_node->super_node = super_node;
    super_node->nodes.push_back(current_node);
    for (Node::Move &next_move : current_node->next_moves_T)
        if (!visited(next_move.node))
            DFS2(next_move.node, super_node);
}

void VoxelGraph::condense_graph()
{
    ++current_visit_id;
    std::stack<Node *> stack;
    for (size_t i = map_volume - 1; i >= map_area; --i)
        if (node_map[i] != nullptr && !visited(node_map[i]))
            DFS1(node_map[i], stack);

    ++current_visit_id;
    while (!stack.empty())
    {
        Node *current_node = stack.top();
        stack.pop();
        if (visited(current_node))
            continue;
        SuperNode *super_node = new SuperNode();
        DFS2(current_node, super_node);
        super_node_list.push_back(super_node);
    }

    for (SuperNode *super_node : super_node_list)
        for (Node *node : super_node->nodes)
            for (Node::Move next_move : node->next_moves)
                if (next_move.node->super_node != super_node)
                {
                    super_node->next_moves.emplace_back(next_move.node->super_node,
                                                        node, next_move.node,
                                                        next_move.direction);
                    next_move.node->super_node->next_moves_T.emplace_back(super_node,
                                                                          next_move.node, node,
                                                                          next_move.direction);
                }
}

Route VoxelGraph::GBFS(Node *source_node, Node *target_node) const
{
    if (source_node == target_node)
        return "";

    source_node->visit_id = current_visit_id;
    using Pair = std::pair<int, Node *>;
    struct PairMinCmp
    {
        bool operator()(Pair a, Pair b) { return a.first > b.first; }
    };
    std::priority_queue<Pair, std::vector<Pair>, PairMinCmp> open_set;

    // track starting neighbors
    for (Node::Move next_move : source_node->next_moves)
    {
        next_move.node->visit_id = current_visit_id;
        next_move.node->last_move = {source_node, next_move.direction};
        open_set.emplace(target_node->coordinate.manhattan_distance(next_move.node->coordinate), next_move.node);
    }

    // search path to target
    while (!open_set.empty())
    {
        Node *current_node = open_set.top().second;
        open_set.pop();

        // check if target has been reached
        if (current_node == target_node)
        {
            Route route;
            while (current_node != source_node)
            {
                route.push_back(current_node->last_move.direction);
                current_node = current_node->last_move.node;
            }
            std::reverse(route.begin(), route.end());
            return route;
        }

        // track new neighbors
        for (Node::Move next_move : current_node->next_moves)
        {
            if (visited(next_move.node))
                continue;
            next_move.node->visit_id = current_visit_id;
            next_move.node->last_move = {current_node, next_move.direction};
            open_set.emplace(target_node->coordinate.manhattan_distance(next_move.node->coordinate), next_move.node);
        }
    }

    // when no path is found
    throw Untraversable(source_node->coordinate, target_node->coordinate);
}

Route VoxelGraph::greedy_best_first_search(const Coordinate &source, const Coordinate &target)
{
    // check source validity
    if (not_in_bounds(source))
        throw InvalidCoordinate(source);
    Node *source_node = node_map[coordinate_to_index(source)];
    if (source_node == nullptr)
        throw InvalidCoordinate(source);

    // check target validity
    if (not_in_bounds(target))
        throw InvalidCoordinate(target);
    Node *target_node = node_map[coordinate_to_index(target)];
    if (target_node == nullptr)
        throw InvalidCoordinate(target);

    ++current_visit_id;

    try
    {
        return GBFS(source_node, target_node);
    }
    catch (const InvalidCoordinate &e)
    {
        throw e;
    }
}

Route VoxelGraph::super_greedy_best_first_search(const Coordinate &source, const Coordinate &target)
{
    // check source validity
    if (not_in_bounds(source))
        throw InvalidCoordinate(source);
    Node *source_node = node_map[coordinate_to_index(source)];
    if (source_node == nullptr)
        throw InvalidCoordinate(source);

    // check target validity
    if (not_in_bounds(target))
        throw InvalidCoordinate(target);
    Node *target_node = node_map[coordinate_to_index(target)];
    if (target_node == nullptr)
        throw InvalidCoordinate(target);

    ++current_visit_id;

    if (source_node->super_node == target_node->super_node)
    {
        try
        {
            return GBFS(source_node, target_node);
        }
        catch (const InvalidCoordinate &e)
        {
            throw e;
        }
    }

    SuperNode *source_super_node = source_node->super_node;
    SuperNode *target_super_node = target_node->super_node;

    source_super_node->visit_id = current_visit_id;

    using Pair = std::pair<int, SuperNode *>;
    std::priority_queue<Pair, std::vector<Pair>, std::greater<Pair>> open_set;

    // track starting neighbors
    for (SuperNode::Move next_move : source_super_node->next_moves)
    {
        next_move.super_node->visit_id = current_visit_id;
        next_move.super_node->last_move = {source_super_node, next_move.exit_node, next_move.entry_node, next_move.direction};
        open_set.emplace(target.manhattan_distance(next_move.entry_node->coordinate), next_move.super_node);
    }

    // search path to target
    while (!open_set.empty())
    {
        SuperNode *current_super_node = open_set.top().second;
        open_set.pop();

        // check if target has been reached
        if (current_super_node == target_super_node)
        {
            Node *current_source_node = current_super_node->last_move.entry_node;
            Route route = GBFS(current_source_node, target_node);
            route.insert(0, 1, current_super_node->last_move.direction);
            Node *current_target_node = current_super_node->last_move.exit_node;
            current_super_node = current_super_node->last_move.super_node;
            while (current_super_node->last_move.super_node != source_super_node)
            {
                current_source_node = current_super_node->last_move.entry_node;
                route.insert(0, GBFS(current_source_node, current_target_node));
                route.insert(0, 1, current_super_node->last_move.direction);
                current_target_node = current_super_node->last_move.exit_node;

                current_super_node = current_super_node->last_move.super_node;
            }
            current_source_node = current_super_node->last_move.entry_node;
            route.insert(0, GBFS(current_source_node, current_target_node));
            route.insert(0, 1, current_super_node->last_move.direction);
            current_target_node = current_super_node->last_move.exit_node;
            route.insert(0, GBFS(source_node, current_target_node));

            return route;
        }

        // track new neighbors
        for (SuperNode::Move next_move : current_super_node->next_moves)
        {
            if (visited(next_move.super_node))
                continue;
            next_move.super_node->visit_id = current_visit_id;
            next_move.super_node->last_move = {current_super_node, next_move.exit_node, next_move.entry_node, next_move.direction};
            open_set.emplace(target.manhattan_distance(next_move.entry_node->coordinate), next_move.super_node);
        }
    }

    // when no path is found
    throw Untraversable(source, target);
}

Route VoxelGraph::RGBFS(Node *source_node, Node *target_node) const
{
    if (source_node == target_node)
        return "";

    // configure current search setup
    source_node->visit_id = current_visit_id;
    using Pair = std::pair<int, Node *>;
    std::priority_queue<Pair, std::vector<Pair>, std::greater<Pair>> open_set;

    // track starting neighbors
    for (Node::Move next_move : source_node->next_moves_T)
    {
        next_move.node->visit_id = current_visit_id;
        next_move.node->last_move = {source_node, next_move.direction};
        open_set.emplace(target_node->coordinate.manhattan_distance(next_move.node->coordinate), next_move.node);
    }

    // search path to target
    while (!open_set.empty())
    {
        Node *current_node = open_set.top().second;
        open_set.pop();

        // check if target has been reached
        if (current_node == target_node)
        {
            Route route;
            while (current_node != source_node)
            {
                route.push_back(current_node->last_move.direction);
                current_node = current_node->last_move.node;
            }
            return route;
        }

        // track new neighbors
        for (Node::Move next_move : current_node->next_moves_T)
        {
            if (visited(next_move.node))
                continue;
            next_move.node->visit_id = current_visit_id;
            next_move.node->last_move = {current_node, next_move.direction};
            open_set.emplace(target_node->coordinate.manhattan_distance(next_move.node->coordinate), next_move.node);
        }
    }

    // when no path is found
    throw Untraversable(source_node->coordinate, target_node->coordinate);
}

Route VoxelGraph::reverse_greedy_best_first_search(const Coordinate &source, const Coordinate &target)
{
    // check source validity
    if (not_in_bounds(target))
        throw InvalidCoordinate(target);
    Node *source_node = node_map[coordinate_to_index(target)];
    if (source_node == nullptr)
        throw InvalidCoordinate(target);

    // check target validity
    if (not_in_bounds(source))
        throw InvalidCoordinate(source);
    Node *target_node = node_map[coordinate_to_index(source)];
    if (target_node == nullptr)
        throw InvalidCoordinate(source);

    ++current_visit_id;

    try
    {
        return RGBFS(source_node, target_node);
    }
    catch (const InvalidCoordinate &e)
    {
        throw e;
    }
}

Route VoxelGraph::reverse_super_greedy_best_first_search(const Coordinate &source, const Coordinate &target)
{
    // check source validity
    if (not_in_bounds(target))
        throw InvalidCoordinate(target);
    Node *source_node = node_map[coordinate_to_index(target)];
    if (source_node == nullptr)
        throw InvalidCoordinate(target);

    // check target validity
    if (not_in_bounds(source))
        throw InvalidCoordinate(source);
    Node *target_node = node_map[coordinate_to_index(source)];
    if (target_node == nullptr)
        throw InvalidCoordinate(source);

    ++current_visit_id;

    if (source_node->super_node == target_node->super_node)
    {
        try
        {
            return RGBFS(source_node, target_node);
        }
        catch (const InvalidCoordinate &e)
        {
            throw e;
        }
    }

    SuperNode *source_super_node = source_node->super_node;
    SuperNode *target_super_node = target_node->super_node;

    source_super_node->visit_id = current_visit_id;

    using Pair = std::pair<int, SuperNode *>;
    std::priority_queue<Pair, std::vector<Pair>, std::greater<Pair>> open_set;

    // track starting neighbors
    for (SuperNode::Move next_move : source_super_node->next_moves_T)
    {
        next_move.super_node->visit_id = current_visit_id;
        next_move.super_node->last_move = {source_super_node, next_move.exit_node, next_move.entry_node, next_move.direction};
        open_set.emplace(target_node->coordinate.manhattan_distance(next_move.entry_node->coordinate), next_move.super_node);
    }

    // search path to target
    while (!open_set.empty())
    {
        SuperNode *current_super_node = open_set.top().second;
        open_set.pop();

        // check if target has been reached
        if (current_super_node == target_super_node)
        {
            Node *current_source_node = current_super_node->last_move.entry_node;
            Route route = GBFS(target_node, current_source_node);
            route += current_super_node->last_move.direction;
            Node *current_target_node = current_super_node->last_move.exit_node;
            current_super_node = current_super_node->last_move.super_node;

            while (current_super_node->last_move.super_node != source_super_node)
            {
                current_source_node = current_super_node->last_move.entry_node;
                route += GBFS(current_target_node, current_source_node);
                route += current_super_node->last_move.direction;
                current_target_node = current_super_node->last_move.exit_node;

                current_super_node = current_super_node->last_move.super_node;
            }
            current_source_node = current_super_node->last_move.entry_node;
            route += GBFS(current_target_node, current_source_node);
            route += current_super_node->last_move.direction;
            current_target_node = current_super_node->last_move.exit_node;
            route += GBFS(current_target_node, source_node);

            return route;
        }

        // track new neighbors
        for (SuperNode::Move next_move : current_super_node->next_moves_T)
        {
            if (visited(next_move.super_node))
                continue;
            next_move.super_node->visit_id = current_visit_id;
            next_move.super_node->last_move = {current_super_node, next_move.exit_node, next_move.entry_node, next_move.direction};
            open_set.emplace(target_node->coordinate.manhattan_distance(next_move.entry_node->coordinate), next_move.super_node);
        }
    }
    HELP;

    // when no path is found
    throw Untraversable(source, target);
}

Route VoxelGraph::bidirectional_best_first_search(const Coordinate &source, const Coordinate &target)
{
    // check source validity
    if (not_in_bounds(source))
        throw InvalidCoordinate(source);
    Node *source_node = node_map[coordinate_to_index(source)];
    if (source_node == nullptr)
        throw InvalidCoordinate(source);

    // trivial case
    if (source == target)
        return "";

    // check target validity
    if (not_in_bounds(target))
        throw InvalidCoordinate(target);
    Node *target_node = node_map[coordinate_to_index(target)];
    if (target_node == nullptr)
        throw InvalidCoordinate(target);

    ++current_visit_id;

    // configure current search setup

    source_node->visit_id = current_visit_id;
    target_node->visit_id_T = current_visit_id;
    using Pair = std::pair<int, Node *>;
    struct PairMinCmp
    {
        bool operator()(Pair a, Pair b) { return a.first > b.first; }
    };
    std::priority_queue<Pair, std::vector<Pair>, PairMinCmp> open_set1, open_set2;

    // track starting neighbors
    for (Node::Move next_move : source_node->next_moves)
    {
        next_move.node->visit_id = current_visit_id;
        next_move.node->last_move = {source_node, next_move.direction};
        open_set1.emplace(target_node->coordinate.manhattan_distance(next_move.node->coordinate), next_move.node);
    }
    for (Node::Move next_move : target_node->next_moves_T)
    {
        next_move.node->visit_id_T = current_visit_id;
        next_move.node->last_move_T = {target_node, next_move.direction};
        open_set2.emplace(source_node->coordinate.manhattan_distance(next_move.node->coordinate), next_move.node);
    }

    // search path to target
    while (!open_set1.empty() && !open_set2.empty())
    {
        Node *current_node1 = open_set1.top().second;
        open_set1.pop();

        Node *current_node2 = open_set2.top().second;
        open_set2.pop();

        // check if target has been reached
        if (current_node2->visit_id == current_visit_id)
        {
            Route route;
            current_node2 = current_node1;
            while (current_node1 != source_node)
            {
                route.push_back(current_node1->last_move.direction);
                current_node1 = current_node1->last_move.node;
            }
            std::reverse(route.begin(), route.end());
            while (current_node2 != target_node)
            {
                route.push_back(current_node2->last_move_T.direction);
                current_node2 = current_node2->last_move_T.node;
            }
            return route;
        }

        // track new neighbors
        for (Node::Move next_move : current_node1->next_moves)
        {
            if (next_move.node->visit_id == current_visit_id)
                continue;
            next_move.node->visit_id = current_visit_id;
            next_move.node->last_move = {current_node1, next_move.direction};
            open_set1.emplace(current_node2->coordinate.manhattan_distance(next_move.node->coordinate), next_move.node);
        }
        for (Node::Move next_move : current_node2->next_moves_T)
        {
            if (next_move.node->visit_id_T == current_visit_id)
                continue;
            next_move.node->visit_id_T = current_visit_id;
            next_move.node->last_move_T = {current_node2, next_move.direction};
            open_set2.emplace(current_node1->coordinate.manhattan_distance(next_move.node->coordinate), next_move.node);
        }
    }

    throw Untraversable(source, target);
}

VoxelGraph::~VoxelGraph()
{
    for (size_t i = 0; i < map_volume; ++i)
        if (node_map[i])
            delete node_map[i];
    delete[] node_map;
    for (SuperNode *super_node : super_node_list)
        delete super_node;
}
