// TODO; constrain time to 99:59:59 (for command line arguments, and when counting up)
// TODO; change time_display to digit_display(with parameterized digit count)
// TODO: web  build
// TODO: android build
// TODO: proper support for wav, and streaming audio from disk
// TODO: more audio formats (mp3, ogg, flac ... would be cool to learn some ffmpeg here)
// TODO: resizable window
// TODO: full set of command line parameters (set time, autostart, loop, bg/fg color, window size/position, gui on/off, etc.)
// TODO: quick count up/down by long press
// TODO: treat the 3 number displays as single line for input (this is really hard to explain -_-)
// TODO: progress bar at the bottom ( flashes when done )
// TODO: loop toggle
// TODO: topbar with recent set timers, with option to pin, displays mini ui(set time and progress bar)
// TODO: run several timers at the same time
// TODO: allow separate alarm sounds per timer, and mix the audio
// TODO: more alarm settings with a visualization (alarm duration, included or excluded from next timer loop)
// TODO: web version (file management)
// TODO: android version (portrait layout, file management)
//
// TODO: functional tests with xdotool and parecord
//		https://askubuntu.com/questions/60837/record-a-programs-output-with-pulseaudio
//		https://ro-che.info/articles/2017-07-21-record-audio-linux
#include <atomic>
#include <cstdio>
#include <sstream>
#include <iomanip>
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
	using support::ston;

	const auto main_color = argc > 5
		? rgb_pixel::from(ston<rgb_pixel::int_type>(argv[5]))
		: 0x009dff_rgb; // or 0x3ba3cd_rgb;

	const auto second_color = argc > 6
		? rgb_pixel::from(ston<rgb_pixel::int_type>(argv[6]))
		: 0x000000_rgb;

	const std::chrono::milliseconds fast_frametime (argc > 7 ? ston<unsigned>(argv[7]) : 33);
	const std::chrono::milliseconds slow_frametime (argc > 8 ? ston<unsigned>(argv[8]) : 256);

	if(main_color == second_color) // would be cool to have a smarter contrast check
	{
		std::fputs("foreground(5th) and background(6th) colors should differ", stderr);
		std::fputs("\n", stderr);
		return -1;
	}
	if(fast_frametime >= slow_frametime)
	{
		std::fputs("fast frame(7th) time should be lower than slow_frametime(8th)", stderr);
		std::fputs("\n", stderr);
		return -2;
	}

	initializer init;
	ui_factory ui;

	auto frametime = fast_frametime;

	std::string icon_string =
		"---------" // "---------"
		"---------" // "-+-----+-"
		"-+-----+-" // "-++---++-"
		"-++---++-" // "-+-+-+-+-"
		"-+-+-+-+-" // "-+--+--+-"
		"-+--+--+-" // "-+-----+-"
		"-+-----+-" // "-+-----+-"
		"---------" // "-+-----+-"
		"---------" // "---------"
	;
	surface icon_small(reinterpret_cast<surface::byte*>(icon_string.data()), {9,9},
		pixel_format(pixel_format::type::index8));
	icon_small.format().palette()->set_color('+', main_color);
	icon_small.format().palette()->set_color('-', 0x0_rgba);

	std::ostringstream title;
	title << std::setfill('0');

	auto music = argc > 1
		?  argv[1][0] != '\0' ? std::optional<musical::wav>(argv[1]) : std::nullopt
		: std::optional<musical::wav>("./melno.wav");

	graphical::software_window win("melno", {400,200});
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
		std::chrono::hours(argc > 2 ? ston<unsigned>(argv[2]) : 0) +
		std::chrono::minutes(argc > 3 ? ston<unsigned>(argv[3]) : 0) +
		std::chrono::seconds(argc > 4 ? ston<unsigned>(argv[4]) : 0),
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


	focus_group main_focus_group{};
	auto focus_handler = [&main_focus_group](auto& element)
	{
		main_focus_group.focus(&element);
	};

	main_focus_group.elements.push_back(&hours_display);
	main_focus_group.elements.push_back(&minutes_display);
	main_focus_group.elements.push_back(&seconds_display);
	hours_display.on_press.push_back(focus_handler);
	minutes_display.on_press.push_back(focus_handler);
	seconds_display.on_press.push_back(focus_handler);

	std::optional<timer::clock::time_point> countup_point = std::nullopt;
	auto& up_button = make_control_button();
	auto& stop_button = make_control_button();
	auto& down_button = make_control_button();

	main_focus_group.elements.push_back(&up_button);
	main_focus_group.elements.push_back(&stop_button);
	main_focus_group.elements.push_back(&down_button);
	up_button.on_press.push_back(focus_handler);
	stop_button.on_press.push_back(focus_handler);
	down_button.on_press.push_back(focus_handler);

	up_button.on_click.push_back([&](auto&)
	{
		countup_point = timer::clock::now() - current_timer.remaining_duration();
	});

	auto reset_current_timer = [&]()
	{
		music_playing = false;
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

	timer stop_button_hold(1s);
	stop_button.on_press.push_back([&stop_button_hold](auto&)
	{
		stop_button_hold = timer(stop_button_hold.duration(), true);
	});

	stop_button.on_release.push_back([&stop_button_hold](auto&)
	{
		stop_button_hold.pause();
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
				[&](window_minimized) { frametime = slow_frametime; },
				[&](window_restored) { frametime = fast_frametime; },
				[](auto) { }
			}, *event);

			main_focus_group.pre_update(*event);
			for(auto&& interactive : ui.interactives())
				interactive->update(*event);
			main_focus_group.post_update(*event);
		}

		if(frametime == fast_frametime)
		{
			fill(win.surface(), bg_color);

			for(auto&& graphic : ui.graphics())
				graphic->draw(win.surface());
			win.update();
		}

		{ using namespace std::chrono;
			auto [h, m, s] =
				split_duration<hours, minutes, seconds>(current_timer.remaining_duration());
			title <<
				std::setw(2) << h.count() <<  ":" <<
				std::setw(2) << m.count() << ":" <<
				std::setw(2) << s.count() << " -- melno";
			win.title(title.str().c_str());
			title.str("");
			title.clear();
		}

		up_button.enable(!music_playing && !countup_point.has_value());
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
				win.restore();
				win.raise();
				frametime = fast_frametime;
				main_focus_group.focus(&stop_button);
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

		if(countup_point)
			current_timer = timer(timer::clock::now() - *countup_point);

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
