#include "testing_tools.hpp"

#include <fstream>

#define HELP PRINT "HELP~main\n"

int main()
{
    // data to stream
    auto data = "worlds/sandwich.vox";
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

/*
    std::ofstream outFile("outputs/raw.bin", std::ios::binary);

    if (!outFile)
    {
        std::cerr << "Error opening file for writing!" << std::endl;
        return 1;
    }

    const char data[] = {0, 1, 2, 3, 5};
    outFile.write(data, sizeof(data));

    outFile.close();
    std::cout << "Data successfully written to file!" << std::endl;
*/
