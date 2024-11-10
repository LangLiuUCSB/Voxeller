#include "testing_tools.hpp"

#include <fstream>

#define HELP PRINT "HELP~test_analysis\n"

int main()
{
    // data to stream
    auto world_name = "goop";
    std::string file_path = "worlds/";
    file_path.append(world_name).append(".vox");
    std::ifstream stream(file_path);
    if (stream.fail())
    {
        ERROR "ERROR: Could not open file: " << file_path << NL;
        return 1;
    }
    PRINT "\nFilepath: " << file_path << NL;

    Chronometer XPOHOMETP;

    // stream to VoxelGraph
    XPOHOMETP.set_hi_res_start();
    VoxelGraph vg(stream);
    XPOHOMETP.set_hi_res_end();
    PRINT "Initialization time: " << XPOHOMETP.get_us() << " microseconds\n\n";

    stream.close();

    // count how many valid positions are there
    size_t num_valid_positions;
    XPOHOMETP.set_hi_res_start();
    num_valid_positions = vg.node_count();
    XPOHOMETP.set_hi_res_end();
    PRINT "Count time: " << XPOHOMETP.get_us() << " microseconds\n"
                         << "This world has " << num_valid_positions
                         << " valid positions.\nThis world has "
                         << num_valid_positions * num_valid_positions
                         << " unique travel plans.\n\n";

    // find max distance
    size_t max_distance;
    XPOHOMETP.set_steady_start();
    max_distance = vg.find_max_distance();
    XPOHOMETP.set_steady_end();
    PRINT "Total pairing time: " << XPOHOMETP.get_ms() << " milliseconds\n"
                                 << "Largest distance between two positions is "
                                 << max_distance << ".\n\n";

    // find all valid positions
    std::vector<Coordinate> positions = vg.find_all_valid_position();

    std::string output_path = "outputs/";
    output_path.append(world_name);
    output_path.append("_positions.txt");
    std::ofstream out_file(output_path);
    if (!out_file)
    {
        std::cerr << "Error opening file for writing!" << std::endl;
        return 1;
    }

    for (size_t i = 0; i < positions.size(); ++i)
        out_file << positions[i].x << ' '
                 << positions[i].y << ' '
                 << positions[i].z << NL;

    return 0;
}
