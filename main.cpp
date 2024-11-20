#include "testing_tools.hpp"

#include <fstream>

#define HELP PRINT "HELP~test_specific\n"

int main()
{
    // data to stream
    auto data = "worlds/platformer.vox";
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

    // set source and target coordinates
    // const Coordinate source(4, 2, 1), target(63, 41, 37); // fortress
    // const Coordinate source(4, 2, 1), target(64, 41, 31); // fortress untraversable
    const Coordinate source(3, 0, 1), target(7, 0, 1);

    /*
    chronometrize(vg, &VoxelGraph::greedy_best_first_search, source, target);

    chronometrize(vg, &VoxelGraph::greedy_best_first_search, source, target);
    chronometrize(vg, &VoxelGraph::reverse_greedy_best_first_search, source, target);

    XPOHOMETP.set_hi_res_start();
    vg.condense_graph();
    XPOHOMETP.set_hi_res_end();
    PRINT "Condensation time: " << XPOHOMETP.get_us() << " microseconds\n\n";

    chronometrize(vg, &VoxelGraph::super_greedy_best_first_search, source, target);
    chronometrize(vg, &VoxelGraph::reverse_super_greedy_best_first_search, source, target);
    */

    chronometrize(vg, &VoxelGraph::bidirectional_best_first_search, source, target);

    PRINT "SUCCESS" << std::endl;
    return 0;
}
