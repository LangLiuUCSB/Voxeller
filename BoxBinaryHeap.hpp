#ifndef BOXBINARYHEAP_HPP
#define BOXBINARYHEAP_HPP
#include <iostream>
#include "BoxArray.hpp"

namespace _2Ls
{
    template <typename T, typename Comparator = std::less<T>>
    class BoxBinaryHeap : public BoxArray<T>
    {
    protected:
        Comparator _cmp;

    public:
        BoxBinaryHeap(const size_t &size = 0, T data[] = nullptr)
            : BoxArray<T>(size, data), _cmp(Comparator()) {}                // Parameterized constructor
        BoxBinaryHeap(std::initializer_list<T> init) : BoxArray<T>(init) {} // Initializer-List constructor
        BoxBinaryHeap(const BoxBinaryHeap &) noexcept = default;            // Copy constructor
        BoxBinaryHeap(BoxBinaryHeap &&) noexcept = default;                 // Move constructor
        BoxBinaryHeap &operator=(const BoxBinaryHeap &) noexcept = default; // Copy assignment
        BoxBinaryHeap &operator=(BoxBinaryHeap &&) noexcept = default;      // Move assignment
        ~BoxBinaryHeap() noexcept = default;                                // Default destructor

        T &at(const size_t &index) override
        {
            if (index >= this->_end)
                throw std::out_of_range("_2Ls::BoxBinaryHeap::at out of range");
            return this->_data[index];
        }
        const T &at(const size_t &index) const override
        {
            if (index >= this->_end)
                throw std::out_of_range("_2Ls::BoxBinaryHeap::at out of range");
            return this->_data[index];
        }
        T &front() override
        {
            if (this->_end == 0)
                throw std::underflow_error("_2Ls::BoxBinaryHeap::front underflow");
            return this->_data[0];
        }
        const T &front() const override
        {
            if (this->_end == 0)
                throw std::underflow_error("_2Ls::BoxBinaryHeap::front underflow");
            return this->_data[0];
        }
        T &back() override
        {
            if (this->_end == 0)
                throw std::underflow_error("_2Ls::BoxBinaryHeap::back underflow");
            return this->_data[this->_end - 1];
        }
        const T &back() const override
        {
            if (this->_end == 0)
                throw std::underflow_error("_2Ls::BoxBinaryHeap::back underflow");
            return this->_data[this->_end - 1];
        }

        void push(const T &value) override
        {
            if (this->_end == this->_max_size)
                throw std::overflow_error("_2Ls::BoxBinaryHeap::push overflow");
            this->_data[this->_end] = value;
            heapify_up(this->_end++);
        }
        void pop() override
        {
            if (this->_end == 0)
                throw std::underflow_error("_2Ls::BoxBinaryHeap::pop underflow");
            this->_data[0] = std::move(this->_data[--this->_end]);
            heapify_down(0);
        }

        T extract() override
        {
            if (this->_end == 0)
                throw std::underflow_error("_2Ls::BoxBinaryHeap::extract underflow");
            T temp = std::move(this->_data[0]);
            this->_data[0] = std::move(this->_data[--this->_end]);
            heapify_down(0);
            return temp;
        }

    protected:
        void heapify_up(size_t current)
        {
            for (size_t parent; current != 0; current = parent)
            {
                if (!_cmp(this->_data[parent = (current - 1) / 2], this->_data[current]))
                    break;
                std::swap(this->_data[parent], this->_data[current]);
            }
        }
        void heapify_down(size_t current)
        {
            for (size_t left, right, priority = current;; current = priority)
            {
                if ((left = 2 * current + 1) < this->_end &&
                    _cmp(this->_data[left], this->_data[priority]))
                    priority = left;
                if ((right = left + 1) < this->_end &&
                    _cmp(this->_data[right], this->_data[priority]))
                    priority = right;
                if (priority == current)
                    break;
                std::swap(this->_data[current], this->_data[priority]);
            }
        }
    };
}

#endif
