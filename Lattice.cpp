#include "Lattice.hpp"

#define PRINT std::cout <<
#define NL '\n'
#define HELP PRINT "HELP~Lattice\n"

#define VOID 7
#define SOLID 4
#define NEW_NODE 2
#define TWO_WAY_NODE 1
#define ONE_WAY_NODE 0

#define UPDATE >>= 1

#define GO_WEST -1
#define GO_NORTH -x_limit
#define GO_DOWN -map_area
#define FALL -= map_area

#define INVERSE -map_area

#define HAS_NEIGHBOR < 4

// static constexpr uint8_t bit_mask[] = {0b1000, 0b0100, 0b0010, 0b0001};
