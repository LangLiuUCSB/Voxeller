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
        catch (const std::exception &e)
        {
            return 1;
        }
        search_times.push_back(search_time);
    }
    PRINT "average search time of basic GBeFS: " << average(search_times) << " microseconds" << NL;

    PRINT "SUCCESS" << std::endl;
    return 0;
}
/*
Average times of search in microseconds:

GBeFS
151.141 150.517 150.468 150.587 150.661
151.260 150.673 150.798 150.301 150.481

RGBeFS
200.725 200.415 204.138 200.477 200.753
204.293 200.983 197.014 195.221 195.121

BDGBeFS (Static Heuristics)
159.873 157.429 157.925 157.206 154.978
153.467 153.869 154.130 155.903 160.377

BGBeFS (Dynamic Heuristics)
146.212 147.226 145.446 145.637 145.585
145.800 147.236 145.660 148.524 146.182
*/
