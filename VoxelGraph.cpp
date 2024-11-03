#include "VoxelGraph.hpp"

#define help std::cout << "help VoxelGraph\n"

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

/*
TODO bidrectional search
TODO fork-join parallelism
TODO path storage and subset search
TODO beachmarking other types of heaps for open_set
TODO fileread async
TODO system dependent space optimization
*/

VoxelGraph::VoxelGraph(std::istream &stream)
{
    stream >> x_limit >> y_limit >> z_limit;
    uint8_t schematic[(map_area = x_limit * y_limit)];

    map_volume = map_area * z_limit;
    node_map = new Node *[map_volume];
    std::memset(node_map, 0, map_volume * sizeof(Node *)); // TODO change 0 to NULL

    std::string row;
    size_t schematic_index = 0;

    // set up schematic lvl 0
    for (int y = 0; y < y_limit; ++y)
    {
        stream >> row;
        for (char character : row)
            for (uint8_t B : bit_mask)
                schematic[schematic_index++] = (hex_to_dec(character) & B) ? SOLID : VOID;
    }
    size_t current_index = map_area;

    auto landing = [this](size_t index)
    {
        while (node_map[index] == nullptr)
            index FALL;
        return index;
    };
    auto directed_link = [this](size_t from, size_t to)
    {
        node_map[from]->nexts.push_back(node_map[to]);
        node_map[to INVERSE]->nexts.push_back(node_map[from INVERSE]);
    };

    // populate node_map
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
    open_set = new BinaryMinHeap(map_volume);
}

VoxelGraph::~VoxelGraph()
{
    for (size_t i = 0; i < map_volume; ++i)
        if (node_map[i])
            delete node_map[i];
    delete[] node_map;
    delete open_set;
}

Path VoxelGraph::find_path(const Coordinate &source, const Coordinate &target)
{
    // check source validity
    if (is_out_of_bounds(source))
        throw InvalidCoordinate(source);
    Node *source_node = node_map[coordinate_to_index(source)];
    if (source_node == nullptr)
        throw InvalidCoordinate(source);

    // trivial case
    if (source == target)
        return Path(0);

    // check target validity
    if (is_out_of_bounds(target))
        throw InvalidCoordinate(target);
    Node *target_node = node_map[coordinate_to_index(target)];
    if (target_node == nullptr)
        throw InvalidCoordinate(target);

    // configure current search setup
    target_node->cost_key = 0;
    auto set_cost = [target](Node *n)
    {
        n->cost_key = abs(target.x - n->coordinate.x) +
                      abs(target.y - n->coordinate.y) +
                      abs(target.z - n->coordinate.z);
    };
    set_cost(source_node);
    source_node->visit_id = ++current_visit_id;
    Node *current_node = source_node;
    auto set_as_visited = [set_cost, &current_node](Node *adjacent_node)
    {
        set_cost(adjacent_node);
        adjacent_node->previous = current_node;
        adjacent_node->prior_move =
            (adjacent_node->coordinate.x - current_node->coordinate.x)
                ? ((adjacent_node->coordinate.x - current_node->coordinate.x == 1)
                       ? Move::EAST
                       : Move::WEST)
                : ((adjacent_node->coordinate.y - current_node->coordinate.y == 1)
                       ? Move::SOUTH
                       : Move::NORTH);
        adjacent_node->visit_id = current_node->visit_id;
    };

    // track possible starting neighbors
    for (Node *adjacent_node : source_node->nexts)
    {
        set_as_visited(adjacent_node);
        open_set->push(adjacent_node);
    }

    // search path to target
    while (!open_set->empty())
    {
        current_node = open_set->pop();

        // check if target has been reached
        if (current_node == target_node)
        {
            open_set->clear();

            // find path length
            size_t path_length = 0;
            for (; current_node != source_node;
                 current_node = current_node->previous)
                ++path_length;

            // retrace path
            Path path(path_length);
            current_node = target_node;
            for (; current_node != source_node;
                 current_node = current_node->previous)
                path.retrace(current_node->prior_move);

            return path;
        }

        // track possible new neighbors
        for (Node *adjacent_node : current_node->nexts)
        {
            if (adjacent_node->visit_id != current_visit_id)
            {
                set_as_visited(adjacent_node);
                open_set->push(adjacent_node);
            }
        }
    }

    // when no path is found
    open_set->clear();
    throw Untraversable(source, target);
}
/*
void VoxelGraph::set_inverse_as_visited(Node *adjacent_node) const
{
    adjacent_node->visit_id = current_visit_id;
    adjacent_node->previous = current_node;
    adjacent_node->prior_move =
        (adjacent_node->coordinate.x - current_node->coordinate.x)
            ? ((adjacent_node->coordinate.x - current_node->coordinate.x == 1)
                   ? Move::WEST
                   : Move::EAST)
            : ((adjacent_node->coordinate.y - current_node->coordinate.y == 1)
                   ? Move::NORTH
                   : Move::SOUTH);
}

Path VoxelGraph::find_path_reverse(const Coordinate &source, const Coordinate &target)
{
    // check source validity
    if (target.x < 0 || target.x >= x_limit ||
        target.y < 0 || target.y >= y_limit ||
        target.z < 0 || target.z >= z_limit ||
        !set_source_node_reverse(target))
        throw InvalidCoordinate(target);

    // trivial case
    if (source == target)
        return Path(0);

    // check target validity
    if (source.x < 0 || source.x >= x_limit ||
        source.y < 0 || source.y >= y_limit ||
        source.z < 0 || source.z >= z_limit ||
        !set_target_node_reverse(source))
        throw InvalidCoordinate(source);

    // configure current search setup
    source_node->visit_id = ++current_visit_id;
    target_node->cost_key = 0;
    auto lambda_set_cost = [source](Node *n)
    { n->cost_key = abs(source.x - n->coordinate.x) +
                    abs(source.y - n->coordinate.y) +
                    abs(source.z - n->coordinate.z); };
    lambda_set_cost(source_node);
    current_node = source_node;

    // track possible starting neighbors
    for (Node *adjacent_node : source_node->nexts)
    {
        // configure current neighbor
        lambda_set_cost(adjacent_node);
        set_inverse_as_visited(adjacent_node);

        // track as searchable
        open_set->push(adjacent_node);
    }

    // search path to target
    while (!open_set->empty())
    {
        current_node = open_set->pop();
        // check if target has been reached
        if (current_node == target_node)
        {
            open_set->clear();

            // find path length
            size_t path_length = 0;
            for (; current_node != source_node; current_node = current_node->previous)
                ++path_length;

            // retrace path
            Path path(path_length);
            current_node = target_node;
            for (; current_node != source_node; current_node = current_node->previous)
                path.retrace(current_node->prior_move);

            return path;
        }

        // track possible new neighbors
        for (Node *adjacent_node : current_node->nexts)
        {
            if (adjacent_node->visit_id != current_visit_id)
            {
                // configure current neighbor
                lambda_set_cost(adjacent_node);
                set_inverse_as_visited(adjacent_node);

                // track as searchable
                open_set->push(adjacent_node);
            }
        }
    }

    open_set->clear();
    throw Untraversable(source, target);
}

Path VoxelGraph::find_path_bidirectional(const Coordinate &source, const Coordinate &target)
{
    // check source validity
    if (source.x < 0 || source.x >= x_limit ||
        source.y < 0 || source.y >= y_limit ||
        source.z < 0 || source.z >= z_limit ||
        !set_source_node(source))
        throw InvalidCoordinate(source);

    // trivial case
    if (source == target)
        return Path(0);

    // check target validity
    if (target.x < 0 || target.x >= x_limit ||
        target.y < 0 || target.y >= y_limit ||
        target.z < 0 || target.z >= z_limit ||
        !set_target_node(target))
        throw InvalidCoordinate(target);

    // configure current search setup
    source_node->visit_id = ++current_visit_id;
    target_node->cost_key = 0;
    auto lambda_set_cost = [target](Node *n)
    {
        n->cost_key = abs(target.x - n->coordinate.x) +
                      abs(target.y - n->coordinate.y) +
                      abs(target.z - n->coordinate.z);
    };
    lambda_set_cost(source_node);
    current_node = source_node;

    // track possible starting neighbors
    for (Node *adjacent_node : source_node->nexts)
    {
        // configure current neighbor
        set_as_visited(adjacent_node);
        lambda_set_cost(adjacent_node);

        // track as searchable
        open_set->push(adjacent_node);
    }

    // search path to target
    while (!open_set->empty())
    {
        current_node = open_set->pop();
        if (current_node->nexts.size() > 4)
        {
            for (auto n : current_node->nexts)
            {
                std::cout << n->coordinate;
            }

            std::cout << current_node->coordinate << "\n";
        }
        // check if target has been reached
        if (current_node == target_node)
        {
            open_set->clear();

            // find path length
            size_t path_length = 0;
            for (; current_node != source_node;
                 current_node = current_node->previous)
                ++path_length;

            // retrace path
            Path path(path_length);
            current_node = target_node;
            for (; current_node != source_node;
                 current_node = current_node->previous)
                path.retrace(current_node->prior_move);

            return path;
        }

        // track possible new neighbors
        for (Node *adjacent_node : current_node->nexts)
        {
            if (adjacent_node->visit_id != current_visit_id)
            {
                // configure current neighbor
                set_as_visited(adjacent_node);
                lambda_set_cost(adjacent_node);

                // track as searchable
                open_set->push(adjacent_node);
            }
        }
    }

    // when no path is found
    open_set->clear();
    throw Untraversable(source, target);
}
*/
