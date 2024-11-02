#ifndef COORDINATE_HPP
#define COORDINATE_HPP

#include <iostream>

struct Coordinate
{
  int x, y, z;
  Coordinate(int x, int y, int z) : x(x), y(y), z(z) {}
  bool operator==(const Coordinate &other) const { return x == other.x && y == other.y && z == other.z; }
};

std::istream &operator>>(std::istream &stream, Coordinate &coordinate);
std::ostream &operator<<(std::ostream &stream, const Coordinate &coordinate);

#endif
