#include "Coordinate.hpp"
#include "Chronometer.hpp"
#include "Lattice.hpp"

#include <iostream>
// #include <fstream>

// LLVM C++ Style Guide Ruler 100 -----------------------------------------------------------------|
int main()
{
    std::ostream &log = std::cout;
    Chronometer X;

    std::string world_name;
    // log << "enter world name: ";
    // std::cin >> world_name;
    world_name = "platformer";

    using FilePath = std::string;
    FilePath file_path = "worlds/" + world_name + ".vox";

    Lattice lattice;

    X.set_hi_res_start();
    try
    {
        lattice = Lattice(file_path);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE
    }
    X.set_hi_res_end();

    log << "Initialization time: " << X.get_us() << " microseconds\n\n";

    log << "SUCCESS" << std::endl;
    return EXIT_SUCCESS;
}
