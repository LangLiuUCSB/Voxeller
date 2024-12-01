#include <iostream>

#include "BoxStack.hpp"
#include "BoxQueue.hpp"
#include "BoxBinaryHeap.hpp"
#include "Chronometer.hpp"
#include "Coordinate.hpp"
#include "Lattice.hpp"
#include "TripPlan.hpp"

// LLVM C++ Style Guide Ruler 100 -----------------------------------------------------------------|
int main()
{
    std::ostream &log = std::cout;
    _2Ls::Chronometer X;

    std::string world_name;
    // log << "enter world name: ";
    // std::cin >> world_name;
    world_name = "junk";
    std::string file_path = "worlds/" + world_name + ".vox";
    log << file_path << "\n\n";

    Lattice L;
    X.set_hi_res_start();
    try
    {
        L = Lattice(file_path);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }
    X.set_hi_res_end();
    log << "Initialization time: " << X.get_us() << " microseconds\n\n";

    X.set_hi_res_start();
    L.condense();
    X.set_hi_res_end();
    log << "Condensation time: " << X.get_us() << " microseconds\n\n";

    char i = Lattice::DFS;
    Lattice::SearchMode mode = static_cast<Lattice::SearchMode>(i);
    for (; i <= Lattice::BIDIRECTIONAL_NEGATIVE_A_STAR; mode = static_cast<Lattice::SearchMode>(++i))
    {
        // X.set_hi_res_start();
        bool valid = L.verify(mode);
        // X.set_hi_res_end();
        // log << "Verification time: " << X.get_us() << " microseconds\n\n";
        if (valid)
            log << int(i) << "good\n";
        else
            log << int(i) << "bad\n";
    }

    /*
    TripPlan trip_plan(Coordinate(7, 0, 9), Coordinate(3, 0, 1)); // a
    Lattice::Route route;
    X.set_hi_res_start();
    try
    {
        route = L.search(trip_plan, Lattice::DFS);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }
    X.set_hi_res_end();
    log << "Route: " << route << ".\n"
        << "Search time: " << X.get_us() << " microseconds\n\n";
    */

    log << "SUCCESS" << std::endl;
    return EXIT_SUCCESS;
}
// LLVM C++ Style Guide Ruler 100 -----------------------------------------------------------------|
