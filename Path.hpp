#ifndef PATH_HPP
#define PATH_HPP

#include <iostream>

#include "Move.hpp"

class Path
{
    size_t num_moves;
    Move *moves;
    size_t current_index;

public:
    Path(size_t num_moves);
    ~Path();
    const Move *begin() const;
    const Move *end() const;
    size_t size() const;
    void retrace(Move move);
    Path backwards() const;
};

std::ostream &operator<<(std::ostream &stream, const Path &path);

#endif
