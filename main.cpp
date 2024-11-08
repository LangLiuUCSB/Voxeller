#include <iostream>
#include <fstream>

#include "VoxelGraph.hpp"

#define PRINT std::cout <<
#define NL '\n'
#define HELP PRINT "HELP~main\n"
#define ERROR std::cerr <<

#include <numeric>

void chronometrize(VoxelGraph &vg,
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
        ERROR "Invalid coordinate: " << e.coordinate << NL;
    }
    catch (const Untraversable &e)
    {
        ERROR "No path from " << e.source << " to " << e.target << NL;
    }
    catch (const std::out_of_range &e)
    {
        ERROR e.what() << NL;
    }
    std::chrono::duration<double, std::micro> elapsed = std::chrono::high_resolution_clock::now() - start;
    PRINT "Search time: " << elapsed.count() << " microseconds.\nPath: " << path << ".\nPath length: " << path.size() << " steps.\n\n";
}

double get_time(VoxelGraph &vg,
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

double average(const std::vector<double> &vec)
{
    if (vec.empty())
        return 0.0;
    return std::accumulate(vec.begin(), vec.end(), 0.0) / vec.size();
}

TravelPlan pop_random(std::vector<TravelPlan> &v)
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

int main()
{
    // data to stream
    auto data = "worlds/fortress.vox";
    std::ifstream stream(data);
    if (stream.fail())
    {
        ERROR "ERROR: Could not open file: " << data << NL;
        return 1;
    }
    PRINT "\nFilepath: " << data << NL;

    std::chrono::time_point<std::chrono::high_resolution_clock> start;
    std::chrono::duration<double, std::micro> elapsed;

    // stream to VoxelGraph
    start = std::chrono::high_resolution_clock::now();
    VoxelGraph vg(stream);
    elapsed = std::chrono::high_resolution_clock::now() - start;
    PRINT "Initialization time: " << elapsed.count() << " microseconds\n\n";

    stream.close();

    // find how many valid positions are there
    size_t num_valid_positions;
    start = std::chrono::high_resolution_clock::now();
    num_valid_positions = vg.node_count();
    elapsed = std::chrono::high_resolution_clock::now() - start;
    PRINT "Count time: " << elapsed.count() << " microseconds\n"
                         << "This world has " << num_valid_positions
                         << " valid positions.\nThis world has " << num_valid_positions * num_valid_positions
                         << " unique travel plans.\n\n";

    // find all valid travel plans
    size_t max_distance;
    start = std::chrono::high_resolution_clock::now();
    max_distance = vg.find_max_distance();
    elapsed = std::chrono::high_resolution_clock::now() - start;
    PRINT "Total pairing time: " << elapsed.count() << " microseconds\n"
                                 << "Largest distance between two positions is " << max_distance
                                 << ".\n\n";

    // find all valid travel plans
    size_t distance = 128;
    std::vector<TravelPlan> travel_plans;
    start = std::chrono::high_resolution_clock::now();
    travel_plans = vg.find_all_travel_plans(distance);
    elapsed = std::chrono::high_resolution_clock::now() - start;
    PRINT "Total pairing time: " << elapsed.count() << " microseconds\n"
                                 << "This world has " << travel_plans.size()
                                 << " unique travel plans that are at least " << distance
                                 << " units apart.\n\n";

    // algorithm benchmarking
    std::vector<double> search_times;
    size_t num_travel_plans = travel_plans.size();
    for (size_t i = 0; i < num_travel_plans; ++i)
    {
        TravelPlan tp = pop_random(travel_plans);
        double search_time;
        try
        {
            search_time = get_time(vg, &VoxelGraph::BDGBeFS, tp.source, tp.target);
        }
        catch (const InvalidCoordinate &e)
        {
            ERROR "Invalid coordinate: " << e.coordinate << NL;
            ERROR "Error from benchmarking" << NL;
            return 1;
        }
        catch (const Untraversable &e)
        {
            ERROR "No path from " << e.source << " to " << e.target << NL;
            ERROR "Error from benchmarking" << NL;
            return 1;
        }
        catch (const std::out_of_range &e)
        {
            ERROR e.what() << NL;
            ERROR "Error from benchmarking" << NL;
            return 1;
        }
        search_times.push_back(search_time);
    }
    PRINT "average search time of reverse GBeFS: " << average(search_times) << " microseconds\n";
    /*
    // set source and target coordinates
    const Coordinate source(4, 2, 1), target(64, 42, 36);

    // find path
    chronometrize(vg, &VoxelGraph::GBeFS, source, target);
    chronometrize(vg, &VoxelGraph::R-GBeFS, source, target); // roughly 7% faster than basic non-reverse GBeFS on average
    chronometrize(vg, &VoxelGraph::BD-GBeFS, source, target); // TODO Error: BinaryHeap overflow caused by push
    chronometrize(vg, &VoxelGraph::EH-BD-GBeFS, source, target); // TODO needs overhaul
    */
    return 0;
}
/*
Average times of search in microseconds:

 GBeFS  151.141 150.517 150.468 150.587 150.661
        151.260 150.673 150.798 150.301 150.481

RGBeFS  139.197 139.096 138.942 139.606 139.778
        139.976 138.997 139.728 141.003 140.184
*/
