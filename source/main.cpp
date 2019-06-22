// TODO: raise window when timer triggers
// TODO: focus behavior and time input
// TODO: progress bar at the bottom
// TODO: topbar with recent set timers, with option to pin, displays mini ui(set time and progress bar)
// TODO: resizable window
// TODO: full set of command line parameters (set time, autostart, loop, bg/fg color, window size/position, gui on/off, etc.)
// TODO: run several timers at the same time
// TODO: allow separate alarm sounds per timer, and mix the audio
// TODO: more alarm settings with a visualization (alarm duration, included or excluded from next timer loop)
#include <atomic>
#include <cstdio>
#include <cerrno>

#include "simple.hpp"
#include "plain_button.h"
#include "layout.h"
#include "digits.h"
#include "ui_factory.hpp"
#include "utils.hpp"
#include "time_display.h"
#include "timer.h"

int main(int argc, const char** argv) try
{
	initializer init;
	ui_factory ui;

	auto music = argc > 1 ? std::optional<musical::wav>(argv[1]) : std::nullopt;

	graphical::software_window win("melno", {400,200});
	auto fg_color = win.surface().format().color(main_color);
	auto bg_color = win.surface().format().color(0x0_rgb);

	rect button_rect{win.size()/8 + int2{5,5}};
	auto make_control_button = [&]() -> auto&
	{
		auto& button = ui.make<plain_button>(fg_color, button_rect);
		return button;
	};

	int2 digit_size{40,100};
	auto digit_spacing = int2::i(5);
	auto make_time_display = [&]() -> auto&
	{
		auto& display = ui.make<time_display>(digit_size, digit_spacing, fg_color);
		return display;
	};

	std::atomic<bool> music_playing = false;
	auto player = [&music_playing, &music, i = loop(music->buffer())](auto& device, auto buffer) mutable
	{
		if(!music_playing)
		{
			i.reset();
			std::fill(buffer.begin(), buffer.end(), device.silence());
			return;
		}

		std::copy_n(i, buffer.size, buffer.begin());

		i += buffer.size;
		if(i == music->buffer().end())
			music_playing = false;
	};
	using music_device = musical::device_with_callback<decltype(player)>;
	std::unique_ptr<music_device> device = nullptr;

	auto current_timer = timer{
		std::chrono::hours(argc > 2 ? support::ston<unsigned>(argv[2]) : 0) +
		std::chrono::minutes(argc > 3 ? support::ston<unsigned>(argv[3]) : 0) +
		std::chrono::seconds(argc > 4 ? support::ston<unsigned>(argv[4]) : 0),
	};

	auto& hours_display = make_time_display();
	auto& minutes_display = make_time_display();
	auto& seconds_display = make_time_display();
	rect separator_rect{{13,100}};
	bounds_layout timer_layout(
		{
			&hours_display,
			&ui.make<digit_bitmap>(digit[10], fg_color, separator_rect),
			&minutes_display,
			&ui.make<digit_bitmap>(digit[10], fg_color, separator_rect),
			&seconds_display
		},
		int2::i(5)
	);
	timer_layout.update();

	std::optional<timer::clock::time_point> countup_point = std::nullopt;
	auto& up_button = make_control_button();
	auto& stop_button = make_control_button();
	auto& down_button = make_control_button();

	up_button.on_click.push_back([&](auto&)
	{
		countup_point = timer::clock::now() - current_timer.remaining_duration();
	});

	stop_button.on_click.push_back([&](auto&)
	{
		if(music_playing)
		{
			music_playing = false;
			device = nullptr;
			current_timer = timer(current_timer.duration());
		}
		else
		{
			if(countup_point)
				countup_point = std::nullopt;
			current_timer.pause();
		}
	});

	down_button.on_click.push_back([&](auto&)
	{
		if(countup_point)
			countup_point = std::nullopt;
		current_timer.resume();
	});

	bounds_layout button_layout ({&up_button, &stop_button, &down_button}, int2::j(5));
	button_layout.update();
	button_layout += int2::j() * center(button_layout, timer_layout);

	bounds_layout main_layout({&timer_layout, &button_layout}, int2::i(15));
	main_layout.update();

	main_layout += center(main_layout, range2D{int2::zero(), win.size()});

	bool done = false;
	while(!done)
	{
		const auto current_time = std::chrono::steady_clock::now();
		using namespace interactive;
		while(auto event = next_event())
		{
			std::visit(support::overloaded{
				[&done](quit_request) { done = true; },
				[](auto) { }
			}, *event);

			for(auto&& interactive : ui.interactives())
				interactive->update(*event);
		}

		fill(win.surface(), bg_color);

		for(auto&& graphic : ui.graphics())
			graphic->draw(win.surface());

		up_button.enable(!music_playing && !countup_point.has_value());
		down_button.enable(!music_playing && current_timer.paused());

		if(current_timer.check())
		{
			if(music && !music_playing)
			{
				music_playing = true;
				device = std::make_unique<music_device>(
					musical::basic_device_parameters{music->obtained()},
					player
				);
				device->play();
			}
			current_timer.pause();
		}

		if(countup_point)
			current_timer = timer(timer::clock::now() - *countup_point);

		seconds_display.set<std::chrono::seconds>(
			minutes_display.set<std::chrono::minutes>(
				hours_display.set<std::chrono::hours>(
					current_timer.remaining_duration()
				)
			)
		);

		win.update();
		std::this_thread::sleep_until(current_time + frametime);
	}

	return 0;
}
catch(...)
{
	if(errno)
		std::perror("ERROR");

	const char* sdl_error = SDL_GetError();
	if(*sdl_error)
		std::puts(sdl_error);

	throw;
}
