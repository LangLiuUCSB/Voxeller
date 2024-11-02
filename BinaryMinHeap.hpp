#ifndef BINARYMINHEAP_HPP
#define BINARYMINHEAP_HPP

#include <algorithm>

#include "Node.hpp"

class BinaryMinHeap
{
    size_t count, capacity;
    Node **heap_array;

    void min_heapify(size_t index);

public:
    BinaryMinHeap(size_t capacity);
    ~BinaryMinHeap();

    void push(Node *node);
    Node *pop();
    bool empty() const;
    void clear();
};

#endif
