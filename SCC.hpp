#ifndef SCC_HPP
#define SCC_HPP

#include "Node.hpp"

class SCC : public std::vector<Node *>
{
    std::vector<SCC *> nexts;
};

#endif
