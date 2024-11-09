#ifndef BINARYHEAP_HPP
#define BINARYHEAP_HPP

#include <algorithm>

template <typename T, typename Comparator>
class BinaryHeap
{
    size_t count, capacity;
    T *heap_array;
    Comparator cmp;

    void min_heapify(size_t index);

public:
    BinaryHeap(size_t capacity, Comparator comparator);
    ~BinaryHeap();

    void push(T t);
    T pop();
    bool empty() const;
    void clear();
};

template <typename T, typename Comparator>
BinaryHeap<T, Comparator>::BinaryHeap(size_t capacity, Comparator comparator)
    : count(0), capacity(capacity), heap_array(new T[capacity]), cmp(comparator) {}

template <typename T, typename Comparator>
BinaryHeap<T, Comparator>::~BinaryHeap() { delete[] heap_array; }

template <typename T, typename Comparator>
void BinaryHeap<T, Comparator>::min_heapify(size_t index)
{
    size_t smallest = index;
    size_t left = 2 * index + 1;
    size_t right = 2 * index + 2;
    if (left < count && cmp(heap_array[left], heap_array[smallest]))
        smallest = left;
    if (right < count && cmp(heap_array[right], heap_array[smallest]))
        smallest = right;
    if (smallest != index)
    {
        std::swap(heap_array[index], heap_array[smallest]);
        min_heapify(smallest);
    }
}

template <typename T, typename Comparator>
void BinaryHeap<T, Comparator>::push(T t)
{
    if (count == capacity)
        throw std::out_of_range("Error: BinaryHeap overflow caused by push\n");
    heap_array[count] = t;
    size_t current_index = count++;
    if (current_index == 0)
        return;
    for (size_t parent_index; current_index != 0; std::swap(heap_array[current_index], heap_array[parent_index]))
    {
        parent_index = (current_index - 1) / 2;
        if (!cmp(heap_array[current_index], heap_array[parent_index]))
            return;
        current_index = parent_index;
    }
}

template <typename T, typename Comparator>
T BinaryHeap<T, Comparator>::pop()
{
    if (count == 0)
        throw std::out_of_range("Error: BinaryHeap is empty and cannot be popped\n");
    T popped_element = heap_array[0];
    heap_array[0] = heap_array[--count];
    min_heapify(0);
    return popped_element;
}

template <typename T, typename Comparator>
bool BinaryHeap<T, Comparator>::empty() const { return count == 0; }

template <typename T, typename Comparator>
void BinaryHeap<T, Comparator>::clear() { count = 0; }

#endif
