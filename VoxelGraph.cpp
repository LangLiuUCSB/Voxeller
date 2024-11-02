#include "VoxelGraph.hpp"

#define help std::cout << "help VoxelGraph\n"

#define VOID 7
#define SOLID 4
#define NEW_NODE 2
#define TWO_WAY_NODE 1
#define ONE_WAY_NODE 0

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

    //* set up schematic lvl 0
    for (int y = 0; y < y_limit; ++y)
    {
        stream >> row;
        for (char character : row)
            for (uint8_t B : bit_mask)
                schematic[schematic_index++] = (hex_to_dec(character) & B) ? SOLID : VOID;
    }

    //* populate node_map
    size_t current_3d_index = map_area;

    auto lambda_west_of = [](size_t index)
    { return --index; };
    auto lambda_north_of = [this](size_t index)
    { return index - x_limit; };
    auto lambda_below = [this](size_t index)
    { return index - map_area; };
    auto lambda_inverse = lambda_below;

    auto lambda_has_neighbor_node = [&schematic](size_t index)
    { return !(schematic[index] & 4); };

    for (int z = 1; z < z_limit; ++z)
    {
        schematic_index = 0;
        for (int y = 0; y < y_limit; ++y)
        {
            stream >> row;
            int x = 0;
            for (char character : row)
            {
                for (
                    uint8_t i = 0;
                    i < 4;
                    ++i, ++current_3d_index, ++schematic_index, ++x)
                {
                    if (hex_to_dec(character) & bit_mask[i])
                        schematic[schematic_index] = SOLID;
                    else // current voxel is not SOLID
                    {
                        if (schematic[schematic_index] == VOID)
                            continue;
                        schematic[schematic_index] >>= 1; // update schematic
                        if (schematic[schematic_index] == NEW_NODE)
                        {
                            Node *new_node = new Node(Coordinate(x, y, z));
                            Node *new_inverse_node = new Node(Coordinate(x, y, z));

                            if (x != 0 && lambda_has_neighbor_node(lambda_west_of(schematic_index)))
                            {
                                size_t neighbor_3d_index = lambda_west_of(current_3d_index);
                                // fall until a Node is found in node_map
                                while (node_map[neighbor_3d_index] == nullptr)
                                    neighbor_3d_index = lambda_below(neighbor_3d_index);

                                new_node->nexts.push_back(node_map[neighbor_3d_index]); // ingressive link

                                node_map[lambda_inverse(neighbor_3d_index)]->nexts.push_back(new_inverse_node); // egressive link

                                if (schematic[lambda_west_of(schematic_index)] != ONE_WAY_NODE)
                                {
                                    node_map[neighbor_3d_index]->nexts.push_back(new_node); // egressive link

                                    new_inverse_node->nexts.push_back(node_map[lambda_inverse(neighbor_3d_index)]); // ingressive link
                                }
                            }
                            if (y != 0 && lambda_has_neighbor_node(lambda_north_of(schematic_index)))
                            {
                                size_t neighbor_3d_index = lambda_north_of(current_3d_index);
                                // fall until a Node is found in node_map
                                while (node_map[neighbor_3d_index] == nullptr)
                                    neighbor_3d_index = lambda_below(neighbor_3d_index);

                                new_node->nexts.push_back(node_map[neighbor_3d_index]); // ingressive link

                                node_map[lambda_inverse(neighbor_3d_index)]->nexts.push_back(new_inverse_node); // egressive link

                                if (schematic[lambda_north_of(schematic_index)] != ONE_WAY_NODE)
                                {
                                    node_map[neighbor_3d_index]->nexts.push_back(new_node); // egressive link

                                    new_inverse_node->nexts.push_back(node_map[lambda_inverse(neighbor_3d_index)]); // ingressive link
                                }
                            }

                            node_map[current_3d_index] = new_node;
                            node_map[lambda_inverse(current_3d_index)] = new_inverse_node;
                        }
                        if (schematic[schematic_index] == TWO_WAY_NODE)
                        {
                            size_t bidirected_node_3d_index = lambda_below(current_3d_index);

                            Node *bidirected_node = node_map[bidirected_node_3d_index];
                            Node *bidirected_inverse_node = node_map[lambda_inverse(bidirected_node_3d_index)];

                            if (x != 0 && schematic[lambda_west_of(schematic_index)] == NEW_NODE)
                            {
                                size_t neighbor_index = lambda_west_of(current_3d_index);

                                bidirected_node->nexts.push_back(node_map[neighbor_index]); // ingressive link
                                node_map[neighbor_index]->nexts.push_back(bidirected_node); // egressive link

                                node_map[lambda_inverse(neighbor_index)]->nexts.push_back(bidirected_inverse_node); // egressive link
                                bidirected_inverse_node->nexts.push_back(node_map[lambda_inverse(neighbor_index)]); // ingressive link
                            }
                            if (y != 0 && schematic[lambda_north_of(schematic_index)] == NEW_NODE)
                            {
                                size_t neighbor_index = lambda_north_of(current_3d_index);

                                bidirected_node->nexts.push_back(node_map[neighbor_index]); // ingressive link
                                node_map[neighbor_index]->nexts.push_back(bidirected_node); // egressive link

                                node_map[lambda_inverse(neighbor_index)]->nexts.push_back(bidirected_inverse_node); // egressive link
                                bidirected_inverse_node->nexts.push_back(node_map[lambda_inverse(neighbor_index)]); // ingressive link
                            }
                        }
                        if (schematic[schematic_index] == ONE_WAY_NODE)
                        {
                            size_t unidirected_node_3d_index = lambda_below(current_3d_index);
                            unidirected_node_3d_index = lambda_below(unidirected_node_3d_index);
                            // fall until a Node is found in node_map
                            while (node_map[unidirected_node_3d_index] == nullptr)
                                unidirected_node_3d_index = lambda_below(unidirected_node_3d_index);

                            Node *unidirected_node = node_map[unidirected_node_3d_index];
                            Node *unidirected_inverse_node = node_map[lambda_inverse(unidirected_node_3d_index)];

                            if (x != 0 && schematic[lambda_west_of(schematic_index)] == NEW_NODE)
                            {
                                size_t neighbor_index = lambda_west_of(current_3d_index);

                                node_map[neighbor_index]->nexts.push_back(unidirected_node); // egressive link

                                unidirected_inverse_node->nexts.push_back(node_map[lambda_inverse(neighbor_index)]); // ingressive link
                            }
                            if (y != 0 && schematic[lambda_north_of(schematic_index)] == NEW_NODE)
                            {
                                size_t neighbor_index = lambda_north_of(current_3d_index);

                                node_map[neighbor_index]->nexts.push_back(unidirected_node); // egressive link

                                unidirected_inverse_node->nexts.push_back(node_map[lambda_inverse(neighbor_index)]); // ingressive link
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

void VoxelGraph::set_as_visited(Node *adjacent_node) const
{
    adjacent_node->prev = current_node;
    adjacent_node->prior_move =
        (adjacent_node->coordinate.x - current_node->coordinate.x)
            ? ((adjacent_node->coordinate.x - current_node->coordinate.x == 1)
                   ? Move::EAST
                   : Move::WEST)
            : ((adjacent_node->coordinate.y - current_node->coordinate.y == 1)
                   ? Move::SOUTH
                   : Move::NORTH);
    adjacent_node->visit_id = current_visit_id;
}

Path VoxelGraph::find_path(const Coordinate &source, const Coordinate &target)
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
                 current_node = current_node->prev)
                ++path_length;

            // retrace path
            Path path(path_length);
            current_node = target_node;
            for (; current_node != source_node;
                 current_node = current_node->prev)
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

void VoxelGraph::set_inverse_as_visited(Node *adjacent_node) const
{
    adjacent_node->visit_id = current_visit_id;
    adjacent_node->prev = current_node;
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
            for (; current_node != source_node; current_node = current_node->prev)
                ++path_length;

            // retrace path
            Path path(path_length);
            current_node = target_node;
            for (; current_node != source_node; current_node = current_node->prev)
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
