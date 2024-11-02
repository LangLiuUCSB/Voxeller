#ifndef ERRORS_H
#define ERRORS_H

#include "Coordinate.hpp"

struct InvalidMoveEnum
{
    uint8_t move_enum;
    InvalidMoveEnum(uint8_t move_enum) : move_enum(move_enum) {}
};

struct InvalidCoordinate
{
    Coordinate coordinate;
    InvalidCoordinate(Coordinate coordinate) : coordinate(coordinate) {}
};

struct Untraversable
{
    Coordinate source;
    Coordinate target;
    Untraversable(Coordinate source, Coordinate target) : source(source), target(target) {}
};

#endif