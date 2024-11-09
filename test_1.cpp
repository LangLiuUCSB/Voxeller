#include "testing_tools.hpp"

#include <fstream>

#define HELP PRINT "HELP~test_random\n"

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

    // find all valid travel plans
    size_t distance = 128;
    std::vector<TravelPlan> travel_plans;
    XPOHOMETP.set_hi_res_start();
    travel_plans = vg.find_all_travel_plans(distance);
    XPOHOMETP.set_hi_res_end();
    PRINT "Total pairing time: " << XPOHOMETP.get_us() << " microseconds\n"
                                 << "This world has " << travel_plans.size()
                                 << " unique travel plans that are at least " << distance
                                 << " units apart.\n\n";

    // algorithm benchmarking
    std::vector<double> search_times;
    size_t num_travel_plans = travel_plans.size();
    for (size_t i = 0; i < num_travel_plans; ++i)
    {
        TravelPlan tp = pop_random(travel_plans);
        double search_time;
        try
        {
            search_time = get_time(vg, &VoxelGraph::BDGBeFS, tp.source, tp.target);
        }
        catch (const InvalidCoordinate &e)
        {
            ERROR "Invalid coordinate: " << e.coordinate << NL;
            ERROR "Error from benchmarking" << NL;
            return 0;
        }
        catch (const Untraversable &e)
        {
            // ERROR "No path from " << e.source << " to " << e.target << NL;
            // ERROR "Error from benchmarking" << NL;
            // return 0;
        }
        catch (const std::out_of_range &e)
        {
            ERROR e.what() << NL;
            ERROR "Error from benchmarking" << NL;
            return 0;
        }
        search_times.push_back(search_time);
    }
    PRINT "average search time of directional GBeFS: " << average(search_times) << " microseconds\n";

    return 0;
}
/*
Average times of search in microseconds:

 GBeFS  151.141 150.517 150.468 150.587 150.661
        151.260 150.673 150.798 150.301 150.481

RGBeFS  139.197 139.096 138.942 139.606 139.778
        139.976 138.997 139.728 141.003 140.184
*/
