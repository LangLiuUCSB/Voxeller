#ifndef TRAVELPLAN_HPP
#define TRAVELPLAN_HPP

#include "Coordinate.hpp"

struct TravelPlan
{
    Coordinate source, target;
    TravelPlan(Coordinate source, Coordinate target) : source(source), target(target) {}
};

#endif
