#ifndef BOXQUEUE_HPP
#define BOXQUEUE_HPP

#include "BoxArray.hpp"

namespace _2Ls
{
    template <typename T>
    class BoxQueue : public BoxArray<T>
    {
        size_t _start = 0;

    public:
        BoxQueue(size_t size) : BoxArray<T>(size) {}              // Parameterized constructor
        BoxQueue() noexcept = default;                            // Default constructor
        BoxQueue(const BoxQueue &) noexcept = default;            // Copy constructor
        BoxQueue(BoxQueue &&) noexcept = default;                 // Move constructor
        BoxQueue &operator=(const BoxQueue &) noexcept = default; // Copy assignment
        BoxQueue &operator=(BoxQueue &&) noexcept = default;      // Move assignment
        ~BoxQueue() noexcept = default;                           // Default destructor

        T *begin() noexcept override { return this->_data + _start; }
        const T *begin() const noexcept override { return this->_data + _start; }

        size_t size() const noexcept override { return this->_end - _start; }
        bool empty() const noexcept override { return this->_end == _start; }

        T &at(const size_t &index) override
        {
            if (index >= this->_end)
                throw std::out_of_range("_2Ls::BoxQueue::at out of range");
            return this->_data[index];
        }
        const T &at(const size_t &index) const override
        {
            if (index >= this->_end)
                throw std::out_of_range("_2Ls::BoxQueue::at out of range");
            return this->_data[index];
        }
        T &front() override
        {
            if (this->_max_size == 0)
                throw std::out_of_range("_2Ls::BoxQueue::front out of range");
            return this->_data[0];
        }
        const T &front() const override
        {
            if (this->_max_size == 0)
                throw std::out_of_range("_2Ls::BoxQueue::front out of range");
            return this->_data[0];
        }
        T &back() override
        {
            if (this->_max_size == 0)
                throw std::out_of_range("_2Ls::BoxQueue::back out of range");
            return this->_data[this->_end - 1];
        }
        const T &back() const override
        {
            if (this->_max_size == 0)
                throw std::out_of_range("_2Ls::BoxQueue::back out of range");
            return this->_data[this->_end - 1];
        }

        void push(const T &value) override
        {
            if (this->_end == this->_max_size)
                throw std::overflow_error("_2Ls::BoxQueue::push overflow");
            this->_data[this->_end++] = value;
        }
        void pop() override
        {
            if (this->_end == _start)
                throw std::underflow_error("_2Ls::BoxQueue::pop overflow");
            ++_start;
        }

        T &extract() override
        {
            if (this->_end == _start)
                throw std::underflow_error("_2Ls::BoxQueue::extract_top underflow");
            return this->_data[_start++];
        }
    };
}

#endif
