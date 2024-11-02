#include "BinaryMinHeap.hpp"

#define help std::cout << "help BinaryMinHeap\n"
#define print std::cout <<
#define newline << "\n";

void BinaryMinHeap::min_heapify(size_t index)
{
    size_t smallest = index;
    size_t left = 2 * index + 1;
    size_t right = 2 * index + 2;
    if (left < count && heap_array[left]->cost_key < heap_array[smallest]->cost_key)
        smallest = left;
    if (right < count && heap_array[right]->cost_key < heap_array[smallest]->cost_key)
        smallest = right;
    if (smallest != index)
    {
        std::swap(heap_array[index], heap_array[smallest]);
        min_heapify(smallest);
    }
}

BinaryMinHeap::BinaryMinHeap(size_t capacity) : count(0), capacity(capacity), heap_array(new Node *[capacity]) {}

BinaryMinHeap::~BinaryMinHeap() { delete[] heap_array; }

void BinaryMinHeap::push(Node *node) // TODO: profile and try inline
{
    if (count == capacity)
    {
        std::cout << "Error: BinaryMinHeap overflow caused by push\n";
        return;
    }
    heap_array[count] = node;
    size_t current_index = count++;
    if (current_index == 0)
        return;
    for (size_t parent_index; current_index != 0; std::swap(heap_array[current_index], heap_array[parent_index]))
    {
        parent_index = (current_index - 1) / 2;
        if (heap_array[current_index]->cost_key >= heap_array[parent_index]->cost_key)
            return;
    }
}

Node *BinaryMinHeap::pop() // TODO: profile try inline
{
    if (count == 0)
        return nullptr;
    Node *popped_node = heap_array[0];
    heap_array[0] = heap_array[--count];
    min_heapify(0);
    return popped_node;
}

bool BinaryMinHeap::empty() const { return count == 0; }

void BinaryMinHeap::clear() { count = 0; }
