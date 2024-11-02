#ifndef UTILS_HPP
#define UTILS_HPP

#include <limits>
#include <new>
#include <istream>

static constexpr uint8_t bit_mask[] = {0b1000, 0b0100, 0b0010, 0b0001};

size_t max_byte_alloc_available();

#endif