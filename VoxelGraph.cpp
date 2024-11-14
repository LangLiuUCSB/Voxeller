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

#define IS_VISITED ->visit_id == current_visit_id

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
    auto directed_link = [this](size_t from, size_t to)
    {
        node_map[from]->nexts.emplace_back(node_map[to]);
        node_map[to INVERSE]->nexts.emplace_back(node_map[from INVERSE]);
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
                            node_map[current_index INVERSE] = new Node(Coordinate(x, y, z));
                            size_t node_index = current_index;
                            if (x != 0 && schematic[schematic_index GO_WEST] HAS_NEIGHBOR)
                            {
                                size_t node_neighbor_index = landing(current_index GO_WEST);
                                directed_link(node_index, node_neighbor_index);
                                if (schematic[schematic_index GO_WEST] != ONE_WAY_NODE)
                                    directed_link(node_neighbor_index, node_index);
                            }
                            if (y != 0 && schematic[schematic_index GO_NORTH] HAS_NEIGHBOR)
                            {
                                size_t node_neighbor_index = landing(current_index GO_NORTH);
                                directed_link(node_index, node_neighbor_index);
                                if (schematic[schematic_index GO_NORTH] != ONE_WAY_NODE)
                                    directed_link(node_neighbor_index, node_index);
                            }
                        }
                        if (schematic[schematic_index] == TWO_WAY_NODE)
                        {
                            size_t node_index = current_index GO_DOWN;
                            if (x != 0 && schematic[schematic_index GO_WEST] == NEW_NODE)
                            {
                                size_t node_neighbor_index = current_index GO_WEST;
                                directed_link(node_index, node_neighbor_index);
                                directed_link(node_neighbor_index, node_index);
                            }
                            if (y != 0 && schematic[schematic_index GO_NORTH] == NEW_NODE)
                            {
                                size_t node_neighbor_index = current_index GO_NORTH;
                                directed_link(node_index, node_neighbor_index);
                                directed_link(node_neighbor_index, node_index);
                            }
                        }
                        if (schematic[schematic_index] == ONE_WAY_NODE)
                        {
                            size_t node_index = landing(current_index GO_DOWN GO_DOWN);
                            if (x != 0 && schematic[schematic_index GO_WEST] == NEW_NODE)
                            {
                                size_t node_neighbor_index = current_index GO_WEST;
                                directed_link(node_neighbor_index, node_index);
                            }
                            if (y != 0 && schematic[schematic_index GO_NORTH] == NEW_NODE)
                            {
                                size_t node_neighbor_index = current_index GO_NORTH;
                                directed_link(node_neighbor_index, node_index);
                            }
                        }
                    }
                }
            }
        }
    }

    // initialize open sets
    open_set1 = new BinaryHeap<Node *, NodePtrMinHeapComparator>(map_volume, NodePtrMinHeapComparator());
    open_set2 = new BinaryHeap<Node *, NodePtrMinHeapComparator>(map_volume, NodePtrMinHeapComparator());
}

void VoxelGraph::make_super_graph()
{
    ++current_visit_id;
    // Find first node in map
    std::stack<Node *> Kosaraju_stack;
    for (size_t i = map_area; i < map_volume; ++i)
    {
        if (node_map[i] == nullptr ||
            node_map[i] IS_VISITED ||
            i != coordinate_to_index(node_map[i]->coordinate))
            continue;
        dfs1(node_map[i], Kosaraju_stack);
    }

    while (!Kosaraju_stack.empty())
    {
        Node *current_node = Kosaraju_stack.top();
        Kosaraju_stack.pop();
        if (node_map[coordinate_to_index(current_node->coordinate) INVERSE] IS_VISITED)
            continue;
        SCC scc;
        dfs2(node_map[coordinate_to_index(current_node->coordinate) INVERSE], scc);
        super_graph.push_back(scc);
    }
    PRINT super_graph.size();
}

size_t VoxelGraph::node_count() const
{
    size_t count = 0;
    for (size_t i = map_area; i < map_volume; ++i)
    {
        if (node_map[i] != nullptr && i == coordinate_to_index(node_map[i]->coordinate))
        {
            ++count;
        }
    }
    return count;
}

std::vector<Coordinate> VoxelGraph::find_all_valid_position() const
{
    std::vector<Coordinate> positions;
    for (size_t i = map_area; i < map_volume; ++i)
    {
        if (node_map[i] != nullptr && i == coordinate_to_index(node_map[i]->coordinate))
        {
            positions.emplace_back(node_map[i]->coordinate);
        }
    }
    return positions;
}

size_t VoxelGraph::find_max_distance() const
{
    size_t max_distance = 0;
    for (size_t i = map_area; i < map_volume; ++i)
    {
        if (node_map[i] != nullptr &&
            i == coordinate_to_index(node_map[i]->coordinate))
        {
            for (size_t j = map_area; j < map_volume; ++j)
            {
                if (node_map[j] != nullptr &&
                    j == coordinate_to_index(node_map[j]->coordinate) &&
                    node_map[i]->coordinate.manhattan_distance(node_map[j]->coordinate) > max_distance)
                {
                    max_distance = node_map[i]->coordinate.manhattan_distance(node_map[j]->coordinate);
                }
            }
        }
    }
    return max_distance;
}

std::vector<TravelPlan> VoxelGraph::find_all_travel_plans(size_t minimum_distance) const
{
    std::vector<TravelPlan> travel_plans;
    for (size_t i = map_area; i < map_volume; ++i)
    {
        if (node_map[i] != nullptr && i == coordinate_to_index(node_map[i]->coordinate))
        {
            for (size_t j = map_area; j < map_volume; ++j)
            {
                if (node_map[j] != nullptr && j == coordinate_to_index(node_map[j]->coordinate) &&
                    node_map[i]->coordinate.manhattan_distance(node_map[j]->coordinate) >= minimum_distance)
                {
                    travel_plans.emplace_back(node_map[i]->coordinate, node_map[j]->coordinate);
                }
            }
        }
    }
    return travel_plans;
}

Route VoxelGraph::GBeFS(const Coordinate &source, const Coordinate &target)
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

    // configure current search setup
    source_node->visit_id = ++current_visit_id;
    target_node->cost_key = 0;
    Node *current_node = source_node;
    auto set_as_visited = [target, &current_node](Node *neighbor_node)
    {
        neighbor_node->previous = current_node;
        neighbor_node->prior_move =
            (neighbor_node->coordinate.x - current_node->coordinate.x)
                ? ((neighbor_node->coordinate.x - current_node->coordinate.x == 1) ? 'e' : 'w')
                : ((neighbor_node->coordinate.y - current_node->coordinate.y == 1) ? 's' : 'n');
        neighbor_node->visit_id = current_node->visit_id;
        neighbor_node->cost_key = target.manhattan_distance(neighbor_node->coordinate);
    };

    // track starting neighbors
    for (Node *neighbor_node : source_node->nexts)
    {
        set_as_visited(neighbor_node);
        open_set1->push(neighbor_node);
    }

    // search path to target
    while (!open_set1->empty())
    {
        current_node = open_set1->pop();

        // check if target has been reached
        if (current_node == target_node)
        {
            // PRINT "Voxelgraph: This GBeFS from " << source << " to " << target << " took " << i << " turns.\n"; // TODO
            open_set1->clear();
            Route route;
            while (current_node != source_node)
            {
                route.push_back(current_node->prior_move);
                current_node = current_node->previous;
            }
            std::reverse(route.begin(), route.end());
            return route;
        }

        // track new neighbors
        for (Node *neighbor_node : current_node->nexts)
        {
            if (neighbor_node IS_VISITED)
                continue;
            set_as_visited(neighbor_node);
            open_set1->push(neighbor_node);
        }
    }

    // when no path is found
    open_set1->clear();
    throw Untraversable(source, target);
}

Route VoxelGraph::RGBeFS(const Coordinate &source, const Coordinate &target)
{
    // check source validity
    if (not_in_bounds(target))
        throw InvalidCoordinate(target);
    Node *source_node = node_map[coordinate_to_index(target) INVERSE];
    if (source_node == nullptr)
        throw InvalidCoordinate(target);

    // trivial case
    if (source == target)
        return "";

    // check target validity
    if (not_in_bounds(source))
        throw InvalidCoordinate(source);
    Node *target_node = node_map[coordinate_to_index(source) INVERSE];
    if (target_node == nullptr)
        throw InvalidCoordinate(source);

    // configure current search setup
    source_node->visit_id = ++current_visit_id;
    target_node->cost_key = 0;
    Node *current_node = source_node;
    auto set_as_visited = [source, &current_node](Node *neighbor_node)
    {
        neighbor_node->previous = current_node;
        neighbor_node->prior_move =
            (neighbor_node->coordinate.x - current_node->coordinate.x)
                ? ((neighbor_node->coordinate.x - current_node->coordinate.x == -1) ? 'e' : 'w')
                : ((neighbor_node->coordinate.y - current_node->coordinate.y == -1) ? 's' : 'n');
        neighbor_node->visit_id = current_node->visit_id;
        neighbor_node->cost_key = source.manhattan_distance(neighbor_node->coordinate);
    };

    // track starting neighbors
    for (Node *neighbor_node : source_node->nexts)
    {
        set_as_visited(neighbor_node);
        open_set1->push(neighbor_node);
    }

    // search path to target
    while (!open_set1->empty())
    {
        current_node = open_set1->pop();

        // check if target has been reached
        if (current_node == target_node)
        {
            // PRINT "Voxelgraph: This Reverse GBeFS from " << source << " to " << target << " took " << i << " turns.\n"; // TODO
            open_set1->clear();
            Route route;
            while (current_node != source_node)
            {
                route.push_back(current_node->prior_move);
                current_node = current_node->previous;
            }
            return route;
        }

        // track new neighbors
        for (Node *neighbor_node : current_node->nexts)
        {
            if (neighbor_node IS_VISITED)
                continue;
            set_as_visited(neighbor_node);
            open_set1->push(neighbor_node);
        }
    }

    // when no path is found
    open_set1->clear();
    throw Untraversable(source, target);
}

Route VoxelGraph::BDGBeFS(const Coordinate &source, const Coordinate &target)
{
    // check source validity
    if (not_in_bounds(source))
        throw InvalidCoordinate(source);
    Node *source_node1 = node_map[coordinate_to_index(source)];
    if (source_node1 == nullptr)
        throw InvalidCoordinate(source);

    // trivial case
    if (source == target)
        return "";

    // check target validity
    if (not_in_bounds(target))
        throw InvalidCoordinate(target);
    Node *target_node1 = node_map[coordinate_to_index(target)];
    if (target_node1 == nullptr)
        throw InvalidCoordinate(target);

    // configure current search setup
    source_node1->visit_id = ++current_visit_id;
    target_node1->cost_key = 0;

    Node *source_node2 = node_map[coordinate_to_index(target) INVERSE];
    Node *target_node2 = node_map[coordinate_to_index(source) INVERSE];

    source_node2->visit_id = current_visit_id;
    target_node2->cost_key = 0;

    Node *current_node1 = source_node1;
    Node *current_node2 = source_node2;

    auto set_as_visited1 = [&current_node2, &current_node1](Node *neighbor_node)
    {
        neighbor_node->visit_id = current_node1->visit_id;
        neighbor_node->previous = current_node1;
        neighbor_node->prior_move =
            (neighbor_node->coordinate.x - current_node1->coordinate.x)
                ? ((neighbor_node->coordinate.x - current_node1->coordinate.x == 1) ? 'e' : 'w')
                : ((neighbor_node->coordinate.y - current_node1->coordinate.y == 1) ? 's' : 'n');
        neighbor_node->cost_key = current_node2->coordinate.manhattan_distance(neighbor_node->coordinate);
    };
    auto set_as_visited2 = [&current_node1, &current_node2](Node *neighbor_node)
    {
        neighbor_node->visit_id = current_node2->visit_id;
        neighbor_node->previous = current_node2;
        neighbor_node->prior_move =
            (neighbor_node->coordinate.x - current_node2->coordinate.x)
                ? ((neighbor_node->coordinate.x - current_node2->coordinate.x == -1) ? 'e' : 'w')
                : ((neighbor_node->coordinate.y - current_node2->coordinate.y == -1) ? 's' : 'n');
        neighbor_node->cost_key = current_node1->coordinate.manhattan_distance(neighbor_node->coordinate);
    };

    // track starting neighbors
    for (Node *neighbor_node : source_node1->nexts)
    {
        set_as_visited1(neighbor_node);
        open_set1->push(neighbor_node);
    }
    for (Node *neighbor_node : source_node2->nexts)
    {
        set_as_visited2(neighbor_node);
        open_set2->push(neighbor_node);
    }

    // search path to target
    while (!open_set1->empty() && !open_set2->empty())
    {
        current_node1 = open_set1->pop();
        current_node2 = open_set2->pop();

        // check if target has been reached
        if (node_map[coordinate_to_index(current_node1->coordinate) INVERSE] IS_VISITED)
        {
            // PRINT "Voxelgraph: This Evolving-Heuristic Bidirectional GBeFS from " << source << " to " << target << " took " << i << " turns.\n"; // TODO
            // PRINT "Voxelgraph: The two searches joined at " << current_node1->coordinate << ".\n";                                               // TODO
            open_set1->clear();
            open_set2->clear();
            Route route;
            current_node2 = node_map[coordinate_to_index(current_node1->coordinate) INVERSE];
            while (current_node1 != source_node1)
            {
                route.push_back(current_node1->prior_move);
                current_node1 = current_node1->previous;
            }
            std::reverse(route.begin(), route.end());
            while (current_node2 != source_node2)
            {
                route.push_back(current_node2->prior_move);
                current_node2 = current_node2->previous;
            }
            return route;
        }

        // track new neighbors
        for (Node *neighbor_node : current_node1->nexts)
        {
            if (neighbor_node IS_VISITED)
                continue;
            set_as_visited1(neighbor_node);
            open_set1->push(neighbor_node);
        }
        for (Node *neighbor_node : current_node2->nexts)
        {
            if (neighbor_node IS_VISITED)
                continue;
            set_as_visited2(neighbor_node);
            open_set2->push(neighbor_node);
        }
    }

    // when no path is found
    open_set1->clear();
    open_set2->clear();
    throw Untraversable(source, target);
}

VoxelGraph::~VoxelGraph()
{
    for (size_t i = 0; i < map_volume; ++i)
        if (node_map[i])
            delete node_map[i];
    delete[] node_map;
    delete open_set1;
    delete open_set2;
}
