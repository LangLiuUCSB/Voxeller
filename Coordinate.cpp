#include "Coordinate.hpp"

std::ostream &operator<<(std::ostream &stream, const Coordinate &coordinate)
{
  return stream << '(' << coordinate.x << ", " << coordinate.y << ", " << coordinate.z << ')';
}
