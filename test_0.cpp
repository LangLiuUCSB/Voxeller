#include "testing_tools.hpp"

#include <fstream>

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

    Chronometer XPOHOMETP;

    // stream to VoxelGraph
    XPOHOMETP.set_hi_res_start();
    VoxelGraph vg(stream);
    XPOHOMETP.set_hi_res_end();
    PRINT "Initialization time: " << XPOHOMETP.get_us() << " microseconds\n\n";

    stream.close();

    // find how many valid positions are there
    size_t num_valid_positions;
    XPOHOMETP.set_hi_res_start();
    num_valid_positions = vg.node_count();
    XPOHOMETP.set_hi_res_end();
    PRINT "Count time: " << XPOHOMETP.get_us() << " microseconds\n"
                         << "This world has " << num_valid_positions
                         << " valid positions.\nThis world has " << num_valid_positions * num_valid_positions
                         << " unique travel plans.\n\n";

    // find all valid travel plans
    size_t max_distance;
    XPOHOMETP.set_hi_res_start();
    max_distance = vg.find_max_distance();
    XPOHOMETP.set_hi_res_end();
    PRINT "Total pairing time: " << XPOHOMETP.get_us() << " microseconds\n"
                                 << "Largest distance between two positions is " << max_distance
                                 << ".\n\n";

    return 0;
}
