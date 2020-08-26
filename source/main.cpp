#include <atomic>
#include <cstdio>
#include <sstream>
#include <iomanip>
#include <cerrno>
#include <iostream>

#include "simple.hpp"
#include "plain_button.h"
#include "layout.h"
#include "digits.h"
#include "ui_factory.hpp"
#include "utils.hpp"
#include "digit_display.h"
#include "timer.h"

const std::chrono::steady_clock::duration max_duration = 99h + 59min + 59s;
const std::chrono::steady_clock::duration min_duration{};

int main(int argc, const char** argv) try
{
	using support::ston;

	const auto main_color = argc > 5
		? rgb_pixel::from(ston<rgb_pixel::int_type>(argv[5]))
		: 0x009dff_rgb; // or 0x3ba3cd_rgb;

	const auto second_color = argc > 6
		? rgb_pixel::from(ston<rgb_pixel::int_type>(argv[6]))
		: 0x000000_rgb;

	const std::chrono::milliseconds frametime (argc > 7 ? ston<unsigned>(argv[7]) : 33);

	if(main_color == second_color) // would be cool to have a smarter contrast check
	{
		std::fputs("foreground and background colors should differ", stderr);
		std::fputs("\n", stderr);
		return -1;
	}

	initializer init;
	ui_factory ui;

	std::string icon_string =
		"---------"  // "---------"
		"+++++++++"  // "-+++++++-"
		"--+------"  // "----+----"
		"--+-+---+"  // "----+----"
		"--+-+-+-+"  // "----+----"
		"--+-+-+-+"  // "----+----"
		"--+-+-+-+"  // "----+----"
		"--+-+++++"  // "----+----"
		"---------"  // "---------"
	;
	surface icon_small(reinterpret_cast<surface::byte*>(icon_string.data()), {9,9},
		pixel_format(pixel_format::type::index8));
	icon_small.format().palette()->set_color('+', main_color);
	icon_small.format().palette()->set_color('-', 0x0_rgba);

	auto music = argc > 1
		?  argv[1][0] != '\0' ? std::optional<musical::wav>(argv[1]) : std::nullopt
		: std::optional<musical::wav>("./truwo.wav");

	graphical::software_window win("truwo", {400,400}, graphical::window::flags::resizable);
	auto fg_color = win.surface().format().color(main_color);
	auto bg_color = win.surface().format().color(second_color);

	surface icon({64,64}, pixel_format(pixel_format::type::rgba8888));
	blit(convert( icon_small, icon.format()), icon, rect{icon.size()});
	win.icon(icon);

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
		auto& display = ui.make<digit_display<>>(digit_size, digit_spacing, fg_color);
		return display;
	};

	std::atomic<bool> music_playing = false;
	auto player = [&music_playing, &music, i = music->buffer().begin()](auto& device, auto buffer) mutable
	{
		if(!music_playing)
		{
			std::fill(buffer.begin(), buffer.end(), device.silence());
			return;
		}

		const auto size = std::min<size_t>(buffer.size, music->buffer().end() - i);
		std::copy_n(i, size, buffer.begin());

		i += size;
		if(i == music->buffer().end())
		{
			i = music->buffer().begin();
			music_playing = false;
		}
	};
	using music_device = musical::device_with_callback<decltype(player)>;
	std::unique_ptr<music_device> device = nullptr;

	auto current_timer = timer{0ms};
	current_timer = timer(clamp(current_timer.duration(), min_duration, max_duration));

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

	hours_display.on_input.push_back([&](auto&&, int old_value, int new_value)
	{
		using namespace std::chrono;
		auto offset = hours(new_value) - hours(old_value);
		current_timer = timer(current_timer.remaining_duration() + offset);
	});
	minutes_display.on_input.push_back([&](auto&&, int old_value, int new_value)
	{
		using namespace std::chrono;
		auto new_minutes = minutes(new_value);
		if(new_minutes >= hours(1))
			new_minutes = hours(1) - minutes(1);
		auto offset = new_minutes - minutes(old_value);
		current_timer = timer(current_timer.remaining_duration() + offset);
	});
	seconds_display.on_input.push_back([&](auto&&, int old_value, int new_value)
	{
		using namespace std::chrono;
		auto new_seconds = seconds(new_value);
		if(new_seconds >= minutes(1))
			new_seconds = minutes(1) - seconds(1);
		auto offset = new_seconds - seconds(old_value);
		current_timer = timer(current_timer.remaining_duration() + offset);
	});


	auto& stop_button = make_control_button();
	auto& down_button = make_control_button();
	focus_vector button_focus_group{
		{&stop_button, &down_button} };

	focus_vector main_focus_group
	{{
		&hours_display,
		&minutes_display,
		&seconds_display,
		&button_focus_group
	}};

	auto focus_handler = [&main_focus_group](auto& element)
	{
		main_focus_group.focus_on(&element);
	};

	stop_button.on_press.push_back(focus_handler);
	down_button.on_press.push_back(focus_handler);
	hours_display.on_press.push_back(focus_handler);
	minutes_display.on_press.push_back(focus_handler);
	seconds_display.on_press.push_back(focus_handler);


	auto reset_current_timer = [&]()
	{
		music_playing = false;
		init.graphics.screensaver.release_one();
		device = nullptr;
		current_timer = timer(current_timer.duration());
	};

	stop_button.on_click.push_back([&](auto&)
	{
		if(music_playing)
		{
			reset_current_timer();
		}
		else
		{
			current_timer.pause();
		}
	});

	down_button.on_click.push_back([&](auto&)
	{
		current_timer.resume();
	});

	timer stop_button_hold(1s);
	stop_button.on_press.push_back([&stop_button_hold](auto&)
	{
		stop_button_hold = timer(stop_button_hold.duration(), true);
	});

	stop_button.on_release.push_back([&stop_button_hold](auto&)
	{
		stop_button_hold.pause();
	});

	bounds_layout button_layout ({&stop_button, &down_button}, int2::j(5));
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
				[&win](window_resized)
				{
					win.update_surface();
				},
				[&win](window_size_changed)
				{
					win.update_surface();
				},
				[&main_focus_group](const mouse_down&)
				{
					main_focus_group.drop_focus();
				},
				[&main_focus_group](const key_pressed& e)
				{
					if(e.data.keycode == keycode::tab)
					{
						auto direction =
							pressed(scancode::rshift) ||
							pressed(scancode::lshift)
								? i_focusable::prev
								: i_focusable::next
						;

						if(!main_focus_group.focus(direction))
						{
							main_focus_group.drop_focus();
							main_focus_group.focus(direction);
						}

					}
				},
				[](auto) { }
			}, *event);

			for(auto&& interactive : ui.interactives())
				interactive->update(*event);
		}

		fill(win.surface(), bg_color);

		for(auto&& graphic : ui.graphics())
			graphic->draw(win.surface());
		win.update();

		down_button.enable(!music_playing && current_timer.paused());
		hours_display.enable(!music_playing);
		minutes_display.enable(!music_playing);
		seconds_display.enable(!music_playing);

		if(stop_button_hold.check())
			reset_current_timer();

		if(current_timer.check())
		{
			if(!music_playing)
			{
				music_playing = true;
				init.graphics.screensaver.keep_alive();
				win.restore();
				win.raise();
				main_focus_group.focus_on(&stop_button);
				if(music)
				{
					device = std::make_unique<music_device>(
						musical::basic_device_parameters{music->obtained()},
						player
					);
					device->play();
				}
			}
			current_timer.pause();
		}

		auto duration = current_timer.remaining_duration();
		hours_display.set(extract_duration<std::chrono::hours>(duration).count());
		minutes_display.set(extract_duration<std::chrono::minutes>(duration).count());
		seconds_display.set(extract_duration<std::chrono::seconds>(duration).count());

		const auto next_frame_time = current_timer.paused()
			? current_time + frametime
			: min(current_timer.target_time_point(), current_time + frametime);
		std::this_thread::sleep_until(next_frame_time);

	}

	return 0;
}
catch(...)
{
	if(errno)
		std::perror("ERROR");

	const char* sdl_error = SDL_GetError();
	if(*sdl_error)
	{
		std::fputs(sdl_error, stderr);
		std::fputs("\n", stderr);
	}

	throw;
}
