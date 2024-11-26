#include "Chronometer.hpp"
#include "Coordinate.hpp"
#include "Lattice.hpp"
#include "TripPlan.hpp"

#include <iostream>

// LLVM C++ Style Guide Ruler 100 -----------------------------------------------------------------|
int main()
{
    std::ostream &log = std::cout;
    _2Ls::Chronometer X;

    std::string world_name;
    // log << "enter world name: ";
    // std::cin >> world_name;
    world_name = "a";
    FilePath file_path = "worlds/" + world_name + ".vox";
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

    for (char i = Lattice::NEGATIVE_GBFS; i < Lattice::JPS; ++i)
    {
        if (L.self_super_check(static_cast<Lattice::SearchMode>(i), Lattice::BIDIRECTIONAL_GBFS))
            log << int(i) << "good\n";
        else
            log << int(i) << "bad\n";
    }

    const TripPlan trip_plan(Coordinate(7, 0, 9), Coordinate(5, 0, 5)); // a
    Lattice::Route route;

    X.set_hi_res_start();
    try
    {
        route = L.super_search(trip_plan, Lattice::REVERSE_GBFS, Lattice::BIDIRECTIONAL_GBFS);
    }
    catch (const std::exception &e)
    {
        route = e.what();
    }
    X.set_hi_res_end();
    log << "Path: " << route << ".\n";
    log << "Search time: " << X.get_us() << " microseconds\n\n";

    log << "SUCCESS" << std::endl;
    return EXIT_SUCCESS;
}
