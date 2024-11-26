#ifndef LATTICEERRORS_H
#define LATTICEERRORS_H

#include "Coordinate.hpp"

class InvalidSource : public std::exception
{
    const std::string message;

public:
    explicit InvalidSource(const Coordinate &coordinate) noexcept
        : message("Invalid source at " + coordinate.to_string()) {}
    const char *what() const noexcept override { return message.c_str(); }
};

class InvalidTarget : public std::exception
{
    const std::string message;

public:
    explicit InvalidTarget(const Coordinate &coordinate) noexcept
        : message("Invalid target at " + coordinate.to_string()) {}
    const char *what() const noexcept override { return message.c_str(); }
};

class Untraversable : public std::exception
{
    const std::string message;

public:
    explicit Untraversable(Coordinate &source, Coordinate &target) noexcept
        : message("No path from " + source.to_string() + " to " + target.to_string()) {}
    const char *what() const noexcept override { return message.c_str(); }
};

class InvalidSearchMode : public std::exception
{
    const std::string message;

public:
    explicit InvalidSearchMode(char enum_constant) noexcept
        : message("Invalid enum constant " + std::string(1, enum_constant)) {}
    const char *what() const noexcept override { return message.c_str(); }
};

class InvalidRoute : public std::exception
{
    const std::string message;

public:
    explicit InvalidRoute(char move, size_t move_index) noexcept
        : message("Invalid route with impossible move \'" + std::string(1, move) + "\' at index " + std::to_string(move_index)) {}
    const char *what() const noexcept override { return message.c_str(); }
};

#endif