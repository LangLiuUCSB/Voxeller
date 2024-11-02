#include "Utils.hpp"

size_t max_byte_alloc_available()
{
    size_t low = 0;
    size_t high = std::numeric_limits<size_t>::max();
    size_t mid;

    // Binary search for the maximum allocatable size
    while (low < high)
    {
        mid = low + (high - low) / 2;
        try
        {
            char *buffer = new char[mid];
            delete[] buffer;
            low = mid + 1; // Allocation successful, try a bigger size
        }
        catch (const std::bad_alloc &)
        {
            high = mid - 1; // Allocation failed, try a smaller size
        }
    }

    return low - 1;
}
