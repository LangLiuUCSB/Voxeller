#include <iostream>
#include <fstream>

#include "VoxelGraph.hpp"

#define PRINT std::cout <<
#define NL '\n'
#define HELP PRINT "HELP~main\n"

#include <numeric>

void chronometrize(VoxelGraph &vg,
                   std::string (VoxelGraph::*f_ptr)(const Coordinate &source, const Coordinate &target),
                   const Coordinate &source,
                   const Coordinate &target)
{
    std::string path;
    auto start = std::chrono::high_resolution_clock::now();
    try
    {
        path = (vg.*f_ptr)(source, target);
    }
    catch (const InvalidCoordinate &err)
    {
        PRINT "Invalid coordinate: " << err.coordinate << NL;
    }
    catch (const Untraversable &err)
    {
        PRINT "No path from " << err.source << " to " << err.target << NL;
    }
    std::chrono::duration<double, std::micro> elapsed = std::chrono::high_resolution_clock::now() - start;
    PRINT "Search time: " << elapsed.count() << " microseconds.\nPath: " << path << ".\nPath length: " << path.size() << " steps.\n\n";
}

double time(VoxelGraph &vg,
            std::string (VoxelGraph::*f_ptr)(const Coordinate &source, const Coordinate &target),
            const Coordinate &source,
            const Coordinate &target)
{
    std::string path;
    auto start = std::chrono::high_resolution_clock::now();
    try
    {
        path = (vg.*f_ptr)(source, target);
    }
    catch (const InvalidCoordinate &err)
    {
        // PRINT "Invalid coordinate: " << err.coordinate << NL;
    }
    catch (const Untraversable &err)
    {
        // PRINT "No path from " << err.source << " to " << err.target << NL;
    }
    std::chrono::duration<double, std::micro> elapsed = std::chrono::high_resolution_clock::now() - start;
    return elapsed.count();
}

double average(const std::vector<double> &vec)
{
    if (vec.empty())
        return 0.0;
    double sum = std::accumulate(vec.begin(), vec.end(), 0.0);
    return sum / vec.size();
}

int main()
{
    // data to stream
    std::string data = "worlds/fortress.vox";
    std::ifstream stream(data);
    if (stream.fail())
    {
        std::cerr << "ERROR: Could not open file: " << data << NL;
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
    for (TravelPlan tp : travel_plans)
    {
        search_times.emplace_back(time(vg, &VoxelGraph::RGBeFS, tp.source, tp.target));
    }
    PRINT "average search time of basic GBeFS: " << average(search_times) << " microseconds\n";

    /*
    // set source and target coordinates
    const Coordinate source(4, 2, 1), target(64, 42, 36);

    // find path
    chronometrize(vg, &VoxelGraph::GBeFS, source, target);
    chronometrize(vg, &VoxelGraph::RGBeFS, source, target);
    chronometrize(vg, &VoxelGraph::BDGBeFS, source, target); // TODO Error: BinaryHeap overflow caused by push
    chronometrize(vg, &VoxelGraph::EHBDGBeFS, source, target); // TODO needs overhaul
    */
    return 0;
}
/*
GBeFS 128.983
RGBeFS 122.934
*/
