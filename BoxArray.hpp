#ifndef BOXARRAY_HPP
#define BOXARRAY_HPP

#include <stddef.h>
#include <iterator>
#include <stdexcept>

namespace _2Ls
{
    template <typename T>
    class BoxArray
    {
    protected:
        size_t _max_size;
        T *_data;
        size_t _end = 0;

    public:
        BoxArray(const size_t &size = 0, T data[] = nullptr) noexcept
            : _max_size(size), _data(data ? data : new T[size]) {} // Parameterized constructor
        BoxArray(std::initializer_list<T> init)
            : _data(new T[init.size()]),
              _max_size(init.size()),
              _end(init.size()) { std::copy(init.begin(), init.end(), _data.get()); } // Initializer-List constructor
        BoxArray(const BoxArray &) noexcept = default;                                // Copy constructor
        BoxArray(BoxArray &&) noexcept = default;                                     // Move constructor
        BoxArray &operator=(const BoxArray &) noexcept = default;                     // Copy assignment
        BoxArray &operator=(BoxArray &&) noexcept = default;                          // Move assignment
        virtual ~BoxArray() { delete[] _data; }                                       // Default destructor

        virtual T *begin() noexcept { return _data; }
        virtual const T *begin() const noexcept { return _data; }
        virtual T *end() noexcept { return _data + _end; }
        virtual const T *end() const noexcept { return _data + _end; }
        virtual std::reverse_iterator<T *> rbegin() noexcept { return std::reverse_iterator<T *>(end()); }
        virtual std::reverse_iterator<const T *> rbegin() const noexcept { return std::reverse_iterator<const T *>(end()); }
        virtual std::reverse_iterator<T *> rend() noexcept { return std::reverse_iterator<T *>(begin()); }
        virtual std::reverse_iterator<const T *> rend() const noexcept { return std::reverse_iterator<const T *>(begin()); }
        virtual const T *cbegin() const noexcept { return begin(); }
        virtual const T *cend() const noexcept { return end(); }
        virtual std::reverse_iterator<const T *> crbegin() const noexcept { return rbegin(); }
        virtual std::reverse_iterator<const T *> crend() const noexcept { return rend(); }

        virtual size_t size() const noexcept { return _end; }
        virtual size_t max_end() const noexcept { return _max_size; }
        virtual bool empty() const noexcept { return _end == 0; }
        virtual bool full() const noexcept { return _end == _max_size; }

        virtual T &operator[](const size_t &index) noexcept { return _data[index]; }
        virtual const T &operator[](const size_t &index) const noexcept { return _data[index]; }
        virtual T &at(const size_t &index)
        {
            if (index >= _end)
                throw std::out_of_range("_2Ls::BoxArray::at out of range");
            return _data[index];
        }
        virtual const T &at(const size_t &index) const
        {
            if (index >= _end)
                throw std::out_of_range("_2Ls::BoxArray::at out of range");
            return _data[index];
        }
        virtual T &front()
        {
            if (_end == 0)
                throw std::underflow_error("_2Ls::BoxArray::front underflow");
            return _data[0];
        }
        virtual const T &front() const
        {
            if (_end == 0)
                throw std::underflow_error("_2Ls::BoxArray::front underflow");
            return _data[0];
        }
        virtual T &back()
        {
            if (_end == 0)
                throw std::underflow_error("_2Ls::BoxArray::back underflow");
            return _data[_end - 1];
        }
        virtual const T &back() const
        {
            if (_end == 0)
                throw std::underflow_error("_2Ls::BoxArray::back underflow");
            return _data[_end - 1];
        }
        virtual T *data() noexcept { return _data; }
        virtual const T *data() const noexcept { return _data; }

        virtual void push(const T &value)
        {
            if (_end == _max_size)
                throw std::overflow_error("_2Ls::BoxArray::push overflow");
            _data[_end++] = value;
        }
        virtual void pop()
        {
            if (_end == 0)
                throw std::underflow_error("_2Ls::BoxArray::pop underflow");
            --_end;
        }

        virtual T extract()
        {
            if (_end == 0)
                throw std::underflow_error("_2Ls::BoxArray::extract underflow");
            return _data[--_end];
        }
        virtual void clear() noexcept { _end = 0; }
    };
}

#endif
