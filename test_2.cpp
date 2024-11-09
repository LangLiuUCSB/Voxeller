#include "testing_tools.hpp"

#define HELP PRINT "HELP~test_specific\n"

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

    // set source and target coordinates
    // const Coordinate source(4, 2, 1), target(64, 42, 36);
    // const Coordinate source(72, 46, 3), target(9, 9, 36); // pop error
    const Coordinate source(7, 3, 36), target(67, 50, 3); // push error

    // find path
    // chronometrize(vg, &VoxelGraph::GBeFS, source, target);
    // chronometrize(vg, &VoxelGraph::RGBeFS, source, target);    // roughly 7% faster than basic non-reverse GBeFS on average
    chronometrize(vg, &VoxelGraph::BDGBeFS, source, target); // TODO Error: BinaryHeap overflow caused by push
    // chronometrize(vg, &VoxelGraph::EHBDGBeFS, source, target); // TODO needs overhaul

    return 0;
}
