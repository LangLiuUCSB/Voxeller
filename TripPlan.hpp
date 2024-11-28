#ifndef TRIPPLAN_HPP
#define TRIPPLAN_HPP

#include "Coordinate.hpp"

struct TripPlan
{
    Coordinate source, target;

    TripPlan(const Coordinate &source, const Coordinate &target) noexcept : source(source), target(target) {} // Parameterized constructor
    TripPlan() noexcept = default;                                                                            // Default constructor
    TripPlan(const TripPlan &) noexcept = default;                                                            // Copy constructor
    TripPlan(TripPlan &&) noexcept = default;                                                                 // Move constructor
    TripPlan &operator=(const TripPlan &) noexcept = default;                                                 // Copy assignment
    TripPlan &operator=(TripPlan &&) noexcept = default;                                                      // Move assignment
    ~TripPlan() noexcept = default;                                                                           // Default destructor
};

#endif
