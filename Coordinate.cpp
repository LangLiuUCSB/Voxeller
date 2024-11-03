#include "Coordinate.hpp"
/*
std::istream &operator>>(std::istream &stream, Coordinate &coordinate)
{
  return stream >> coordinate.x >> coordinate.y >> coordinate.z;
}
*/
std::ostream &operator<<(std::ostream &stream, const Coordinate &coordinate)
{
  return stream << '(' << coordinate.x << ", " << coordinate.y << ", " << coordinate.z << ')';
}
