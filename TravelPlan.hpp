#ifndef TRAVELPLAN_HPP
#define TRAVELPLAN_HPP

#include "Coordinate.hpp"

#define ERROR std::cerr <<

struct TravelPlan
{
    Coordinate source, target;
    TravelPlan(Coordinate source, Coordinate target) : source(std::move(source)), target(std::move(target)) {}
    TravelPlan(TravelPlan &&other) noexcept : source(std::move(other.source)), target(std::move(other.target)) {}
    TravelPlan &operator=(TravelPlan &&other) noexcept
    {
        if (this != &other)
        {
            source = std::move(other.source);
            target = std::move(other.target);
        }
        return *this;
    }
    TravelPlan(const TravelPlan &) = delete;
    TravelPlan &operator=(const TravelPlan &) = delete;
};

#endif
