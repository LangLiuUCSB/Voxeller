#ifndef BOXSTACK_HPP
#define BOXSTACK_HPP

#include "BoxArray.hpp"

namespace _2Ls
{
    template <typename T>
    class BoxStack : public BoxArray<T>
    {
    public:
        BoxStack(size_t size) : BoxArray<T>(size) {}              // Parameterized constructor
        BoxStack() noexcept = default;                            // Default constructor
        BoxStack(const BoxStack &) noexcept = default;            // Copy constructor
        BoxStack(BoxStack &&) noexcept = default;                 // Move constructor
        BoxStack &operator=(const BoxStack &) noexcept = default; // Copy assignment
        BoxStack &operator=(BoxStack &&) noexcept = default;      // Move assignment
        ~BoxStack() noexcept = default;                           // Default destructor

        T &at(const size_t &index) override
        {
            if (index >= this->_end)
                throw std::out_of_range("_2Ls::BoxStack::at out of range");
            return this->_data[index];
        }
        const T &at(const size_t &index) const override
        {
            if (index >= this->_end)
                throw std::out_of_range("_2Ls::BoxStack::at out of range");
            return this->_data[index];
        }
        T &front() override
        {
            if (this->_max_size == 0)
                throw std::out_of_range("_2Ls::BoxStack::front out of range");
            return this->_data[0];
        }
        const T &front() const override
        {
            if (this->_max_size == 0)
                throw std::out_of_range("_2Ls::BoxStack::front out of range");
            return this->_data[0];
        }
        T &back() override
        {
            if (this->_max_size == 0)
                throw std::out_of_range("_2Ls::BoxStack::back out of range");
            return this->_data[this->_end - 1];
        }
        const T &back() const override
        {
            if (this->_max_size == 0)
                throw std::out_of_range("_2Ls::BoxStack::back out of range");
            return this->_data[this->_end - 1];
        }

        void push(const T &value) override
        {
            if (this->_end == this->_max_size)
                throw std::overflow_error("_2Ls::BoxStack::push overflow");
            this->_data[this->_end++] = value;
        }
        void pop() override
        {
            if (this->_end == 0)
                throw std::underflow_error("_2Ls::BoxStack::pop overflow");
            --this->_end;
        }

        T &extract() override
        {
            if (this->_end == 0)
                throw std::underflow_error("_2Ls::BoxStack::extract_top underflow");
            return this->_data[--this->_end];
        }
    };
}

#endif
