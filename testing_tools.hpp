#ifndef TESTING_TOOLS_HPP
#define TESTING_TOOLS_HPP

#include "VoxelGraph.hpp"
#include "Chronometer.hpp"

#include <numeric>

#define PRINT std::cout <<
#define NL '\n'
#define ERROR std::cerr <<

inline void chronometrize(VoxelGraph &vg,
                          Route (VoxelGraph::*f_ptr)(const Coordinate &source, const Coordinate &target),
                          const Coordinate &source,
                          const Coordinate &target)
{
    Chronometer XPOHOMETP;
    Route path;
    XPOHOMETP.set_hi_res_start();
    try
    {
        path = (vg.*f_ptr)(source, target);
    }
    catch (const InvalidCoordinate &e)
    {
        std::cerr << "Invalid coordinate: " << e.coordinate << '\n';
    }
    catch (const Untraversable &e)
    {
        std::cerr << "No path from " << e.source << " to " << e.target << '\n';
    }
    catch (const std::out_of_range &e)
    {
        std::cerr << e.what() << '\n';
    }
    XPOHOMETP.set_hi_res_end();
    std::cout << "Search time: " << XPOHOMETP.get_us() << " microseconds.\nPath: " << path << ".\nPath length: " << path.size() << " steps.\n\n";
}

inline double get_time(VoxelGraph &vg,
                       Route (VoxelGraph::*f_ptr)(const Coordinate &source, const Coordinate &target),
                       const Coordinate &source,
                       const Coordinate &target)
{
    Chronometer XPOHOMETP;
    Route path;
    try
    {
        XPOHOMETP.set_hi_res_start();
        path = (vg.*f_ptr)(source, target);
    }
    catch (const InvalidCoordinate &e)
    {
        ERROR "Invalid coordinate: " << e.coordinate << NL;
        throw e;
    }
    catch (const Untraversable &e)
    {
        XPOHOMETP.set_hi_res_end();
        return XPOHOMETP.get_us();
    }
    catch (const std::out_of_range &e)
    {
        ERROR e.what() << NL;
        throw e;
    }
    XPOHOMETP.set_hi_res_end();
    return XPOHOMETP.get_us();
}

inline double average(const std::vector<double> &vec) { return vec.empty() ? 0.0 : std::accumulate(vec.begin(), vec.end(), 0.0) / vec.size(); }

inline TravelPlan pop_random(std::vector<TravelPlan> &v)
{
    if (v.empty())
        throw std::out_of_range("Cannot pop from an empty vector\n");
    std::srand(static_cast<unsigned int>(std::time(0)));
    int random_index = std::rand() % v.size();
    TravelPlan popped_element = std::move(v[random_index]);
    std::swap(v[random_index], v.back());
    v.pop_back();
    return popped_element;
}

#endif