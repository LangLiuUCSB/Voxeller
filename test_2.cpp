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
    const Coordinate source(0, 11, 1), target(15, 0, 1);

    // find path
    // chronometrize(vg, &VoxelGraph::GBeFS, source, target);
    // chronometrize(vg, &VoxelGraph::RGBeFS, source, target);
    chronometrize(vg, &VoxelGraph::BDGBeFS, source, target);

    PRINT "SUCCESS" << std::endl;
    return 0;
}
