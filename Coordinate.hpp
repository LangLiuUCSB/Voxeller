#ifndef COORDINATE_HPP
#define COORDINATE_HPP

#include <ostream>

struct Coordinate
{
  int x, y, z;
  Coordinate(const int x, const int y, const int z) noexcept : x(x), y(y), z(z) {} // Parameterized constructor
  Coordinate() noexcept : x(0), y(0), z(0) {}                                      // Default constructor
  Coordinate(const Coordinate &) noexcept = default;                               // Copy constructor
  Coordinate(Coordinate &&) noexcept = default;                                    // Move constructor
  Coordinate &operator=(const Coordinate &) noexcept = default;                    // Copy assignment
  Coordinate &operator=(Coordinate &&) noexcept = default;                         // Move assignment
  ~Coordinate() noexcept = default;                                                // Destructor
  bool operator==(const Coordinate &other) const noexcept
  {
    return x == other.x && y == other.y && z == other.z;
  }
  bool operator!=(const Coordinate &other) const noexcept
  {
    return x != other.x || y != other.y || z != other.z;
  }
  Coordinate east() const noexcept { return Coordinate(x + 1, y, z); }
  Coordinate west() const noexcept { return Coordinate(x - 1, y, z); }
  Coordinate south() const noexcept { return Coordinate(x, y + 1, z); }
  Coordinate north() const noexcept { return Coordinate(x, y - 1, z); }
  Coordinate up() const noexcept { return Coordinate(x, y, z + 1); }
  Coordinate down() const noexcept { return Coordinate(x, y, z - 1); }
  void fall() noexcept { --z; }
  std::string to_string() const noexcept
  {
    return "(" + std::to_string(x) + "," + std::to_string(y) + "," + std::to_string(z) + ")";
  }
};

inline std::ostream &operator<<(std::ostream &s, const Coordinate &c) noexcept
{
  return s << c.to_string();
}
inline size_t manhattan_distance(const Coordinate &c1, const Coordinate &c2) noexcept
{
  return abs(c1.x - c2.x) + abs(c1.y - c2.y) + abs(c1.z - c2.z);
}
struct CoordinateHash
{
  std::size_t operator()(const Coordinate &c) const noexcept
  {
    return c.x * 73856093 ^ c.y * 19349663 ^ c.z * 83492791;
  }
};

#endif
