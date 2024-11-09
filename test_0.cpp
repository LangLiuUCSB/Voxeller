#include "testing_tools.hpp"

#define HELP PRINT "HELP~test_analysis\n"

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

    return 0;
}
