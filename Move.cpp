#include "Move.hpp"

std::ostream &operator<<(std::ostream &stream, Move move)
{
    switch (move)
    {
    case Move::NORTH:
        return stream << 'n';
    case Move::EAST:
        return stream << 'e';
    case Move::SOUTH:
        return stream << 's';
    case Move::WEST:
        return stream << 'w';
    }
    throw InvalidMoveEnum(move);
}