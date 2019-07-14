#ifndef SIMPLE_HPP
#define SIMPLE_HPP

#include <thread>
#include <chrono>

#include "simple/support.hpp"
#include "simple/geom.hpp"
#include "simple/graphical.hpp"
#include "simple/interactive.hpp"
#include "simple/musical.hpp"

using namespace simple;

using graphical::int2;
using graphical::float2;
using graphical::rect;
using graphical::anchored_rect;
using graphical::rgb_pixel;
using graphical::rgb_vector;
using range2D = rect::range;
using simple::support::rational;
using ratio2 = rational<int2>;

class initializer
{
	graphical::initializer graphics;
	interactive::initializer interactions;
	musical::initializer music;
};

using namespace std::literals;
using namespace graphical::color_literals;
using graphical::surface;
using graphical::pixel_format;

constexpr range2D invalid_range{std::numeric_limits<int2>::max(),std::numeric_limits<int2>::min()};

#endif /* end of include guard */
