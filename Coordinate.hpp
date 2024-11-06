#ifndef COORDINATE_HPP
#define COORDINATE_HPP

#include <ostream>

struct Coordinate
{
  const int x, y, z;
  Coordinate(int x, int y, int z) : x(x), y(y), z(z) {}
  bool operator==(const Coordinate &other) const { return x == other.x && y == other.y && z == other.z; }
  size_t manhattan_distance(const Coordinate &other) const { return abs(x - other.x) + abs(y - other.y) + abs(z - other.z); }
};

inline std::ostream &operator<<(std::ostream &stream, const Coordinate &coordinate) { return stream << '(' << coordinate.x << ", " << coordinate.y << ", " << coordinate.z << ')'; }

#endif
