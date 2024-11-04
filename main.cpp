#include <iostream>
#include <fstream>

#include "VoxelGraph.hpp"

#define PRINT std::cout <<
#define NL '\n'
#define HELP PRINT "HELP~main\n"

void chronometrize(VoxelGraph &map,
                   std::string (VoxelGraph::*f_ptr)(const Coordinate &source, const Coordinate &target),
                   const Coordinate &source,
                   const Coordinate &target)
{
    std::string path;
    auto start = std::chrono::high_resolution_clock::now();
    try
    {
        path = (map.*f_ptr)(source, target);
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

    // stream to VoxelGraph
    auto start = std::chrono::high_resolution_clock::now();
    VoxelGraph map(stream);
    std::chrono::duration<double, std::micro> elapsed = std::chrono::high_resolution_clock::now() - start;
    PRINT "Initialization time: " << elapsed.count() << " microseconds\n\n";

    stream.close();

    // set source and target coordinates
    const Coordinate source(4, 2, 1), target(64, 42, 36);

    // find path
    chronometrize(map, &VoxelGraph::GBeFS, source, target);
    chronometrize(map, &VoxelGraph::RGBeFS, source, target);
    chronometrize(map, &VoxelGraph::BDGBeFS, source, target);
    chronometrize(map, &VoxelGraph::EHBDGBeFS, source, target); // TODO needs overhaul

    return 0;
}