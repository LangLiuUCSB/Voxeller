#ifndef TESTING_TOOLS_HPP
#define TESTING_TOOLS_HPP

#include <iostream>
#include <fstream>

#include "VoxelGraph.hpp"

#include <numeric>

#define PRINT std::cout <<
#define NL '\n'
#define ERROR std::cerr <<

inline void chronometrize(VoxelGraph &vg,
                          Route (VoxelGraph::*f_ptr)(const Coordinate &source, const Coordinate &target),
                          const Coordinate &source,
                          const Coordinate &target)
{
    Route path;
    auto start = std::chrono::high_resolution_clock::now();
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
    std::chrono::duration<double, std::micro> elapsed = std::chrono::high_resolution_clock::now() - start;
    std::cout << "Search time: " << elapsed.count() << " microseconds.\nPath: " << path << ".\nPath length: " << path.size() << " steps.\n\n";
}

inline double get_time(VoxelGraph &vg,
                       Route (VoxelGraph::*f_ptr)(const Coordinate &source, const Coordinate &target),
                       const Coordinate &source,
                       const Coordinate &target)
{
    Route path;
    auto start = std::chrono::high_resolution_clock::now();
    path = (vg.*f_ptr)(source, target);
    std::chrono::duration<double, std::micro> elapsed = std::chrono::high_resolution_clock::now() - start;
    return elapsed.count();
}

inline double average(const std::vector<double> &vec)
{
    if (vec.empty())
        return 0.0;
    return std::accumulate(vec.begin(), vec.end(), 0.0) / vec.size();
}

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