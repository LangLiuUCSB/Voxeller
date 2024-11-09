#include "testing_tools.hpp"

#include <fstream>

#define HELP PRINT "HELP~main\n"

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

    return 0;
}