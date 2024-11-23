#ifndef ERRORS_H
#define ERRORS_H

#include "Coordinate.hpp"

struct InvalidCoordinate
{
    const Coordinate coordinate;
    InvalidCoordinate(Coordinate &coordinate) noexcept : coordinate(coordinate) {}
};

struct Untraversable
{
    const Coordinate source, target;
    Untraversable(Coordinate &source, Coordinate &target) noexcept : source(source), target(target) {}
};

#endif