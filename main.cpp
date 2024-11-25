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
    world_name = "goop";
    FilePath file_path = "worlds/" + world_name + ".vox";
    log << file_path << "\n\n";

    Lattice lattice;
    X.set_hi_res_start();
    try
    {
        lattice = Lattice(file_path);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }
    X.set_hi_res_end();
    log << "Initialization time: " << X.get_us() << " microseconds\n\n";

    // const TripPlan trip_plan(Coordinate(3, 0, 1), Coordinate(7, 0, 1)); // platformer
    // const TripPlan trip_plan(Coordinate(4, 2, 1), Coordinate(63, 41, 37)); // fortress
    const TripPlan trip_plan(Coordinate(32, 64, 51), Coordinate(48, 14, 39)); // goop
    Lattice::Route route;
    X.set_hi_res_start();

    try
    {
        route = lattice.search(trip_plan, Lattice::GBFS);
    }
    catch (const std::exception &e)
    {
        route = e.what();
    }
    X.set_hi_res_end();
    log << "Path: " << route << ".\n";
    route.clear();
    log << "Search time: " << X.get_us() << " microseconds\n\n";

    X.set_hi_res_start();
    lattice.condense();
    X.set_hi_res_end();
    log << "Condensation time: " << X.get_us() << " microseconds\n\n";

    X.set_hi_res_start();
    try
    {
        route = lattice.super_search(trip_plan, Lattice::REVERSE_GBFS, Lattice::REVERSE_GBFS);
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
// LLVM C++ Style Guide Ruler 100 -----------------------------------------------------------------|
