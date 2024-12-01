#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

constexpr char mask_bits[] = {0b1000, 0b0100, 0b0010, 0b0001};

constexpr int int_most_significant_bit = (sizeof(int) * __CHAR_BIT__ - 1);

#endif