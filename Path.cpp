#include "Path.hpp"

Path::Path(size_t num_moves) : num_moves(num_moves), moves(new Move[num_moves]), current_index(num_moves - 1) {}
Path::~Path() { delete[] moves; }
const Move *Path::begin() const { return moves; }
const Move *Path::end() const { return moves + num_moves; }
size_t Path::size() const { return num_moves; }
void Path::retrace(Move move)
{
    if (current_index != SIZE_MAX)
        moves[current_index--] = move;
    else
        std::cout << "Error: Path Overflow\n";
}

Path Path::backwards() const
{
    Path path_backwards(num_moves);
    for (size_t i = 0; i < num_moves;)
        path_backwards.retrace(moves[i++]);
    return path_backwards;
}

std::ostream &operator<<(std::ostream &stream, const Path &path)
{
    for (Move move : path)
        stream << move;
    return stream;
}
