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

class initializer
{
	graphical::initializer graphics;
	interactive::initializer interactions;
	musical::initializer music;
};

using namespace std::literals;
using namespace graphical::color_literals;

// rgb_pixel main_color = 0x3ba3cd_rgb;
const rgb_pixel main_color = 0x009dff_rgb;
const auto frametime = 33ms;

#endif /* end of include guard */