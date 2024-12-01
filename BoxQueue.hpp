#ifndef BOXQUEUE_HPP
#define BOXQUEUE_HPP

#include "BoxArray.hpp"

namespace _2Ls
{
    template <typename T>
    class BoxQueue : public BoxArray<T>
    {
    protected:
        size_t _start = 0;

    public:
        BoxQueue(const size_t &size = 0, T data[] = nullptr)
            : BoxArray<T>(size, data) {}                               // Parameterized constructor
        BoxQueue(std::initializer_list<T> init) : BoxArray<T>(init) {} // Initializer-List constructor
        BoxQueue(const BoxQueue &) noexcept = default;                 // Copy constructor
        BoxQueue(BoxQueue &&) noexcept = default;                      // Move constructor
        BoxQueue &operator=(const BoxQueue &) noexcept = default;      // Copy assignment
        BoxQueue &operator=(BoxQueue &&) noexcept = default;           // Move assignment
        ~BoxQueue() noexcept = default;                                // Default destructor

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
            if (this->_end - _start == 0)
                throw std::underflow_error("_2Ls::BoxQueue::front underflow");
            return this->_data[0];
        }
        const T &front() const override
        {
            if (this->_end - _start == 0)
                throw std::underflow_error("_2Ls::BoxQueue::front underflow");
            return this->_data[0];
        }
        T &back() override
        {
            if (this->_end - _start == 0)
                throw std::underflow_error("_2Ls::BoxQueue::back underflow");
            return this->_data[this->_end - 1];
        }
        const T &back() const override
        {
            if (this->_end - _start == 0)
                throw std::underflow_error("_2Ls::BoxQueue::back underflow");
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
                throw std::underflow_error("_2Ls::BoxQueue::pop underflow");
            ++_start;
        }

        T extract() override
        {
            if (this->_end == _start)
                throw std::underflow_error("_2Ls::BoxQueue::extract underflow");
            return this->_data[_start++];
        }
        void clear() noexcept override { _start = 0, this->_end = 0; }
    };
}

#endif
