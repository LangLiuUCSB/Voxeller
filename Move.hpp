#ifndef MOVE_HPP
#define MOVE_HPP

#include <ostream>
#include <cstdint>

#include "Errors.hpp"

enum Move : uint8_t
{
  EAST,
  SOUTH,
  WEST,
  NORTH,
};

static constexpr Move cardinal_directions[] = {Move::EAST, Move::SOUTH, Move::WEST, Move::NORTH};

std::ostream &operator<<(std::ostream &stream, Move move);

#endif
