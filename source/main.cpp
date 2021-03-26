#include <atomic>
#include <cstdio>
#include <iomanip>
#include <cerrno>

#include "simple.hpp"
#include "plain_button.h"
#include "layout.h"
#include "digits.h"
#include "ecs.hpp"
#include "utils.hpp"
#include "digit_display.h"
#include "timer.h"

// const std::chrono::steady_clock::duration max_duration = 99h + 59min + 59s;
// const std::chrono::steady_clock::duration min_duration{};

struct nothing {};
using movement = motion::movement<std::chrono::steady_clock::duration, nothing, nothing>;

int main(int argc, const char** argv) try
{
	using support::ston;

	const auto main_color = argc > 2
		? rgb_pixel::from(ston<rgb_pixel::int_type>(argv[2]))
		: 0x009dff_rgb; // or 0x3ba3cd_rgb;

	const auto second_color = argc > 3
		? rgb_pixel::from(ston<rgb_pixel::int_type>(argv[3]))
		: 0x000000_rgb;

	const std::chrono::milliseconds frametime (argc > 4 ? ston<unsigned>(argv[4]) : 33);

	if(main_color == second_color) // would be cool to have a smarter contrast check
	{
		std::fputs("foreground and background colors should differ", stderr);
		std::fputs("\n", stderr);
		return -1;
	}

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

	initializer init;

	surface icon_small(reinterpret_cast<surface::byte*>(icon_string.data()), {9,9},
		pixel_format(pixel_format::type::index8));
	icon_small.format().palette()->set_color('+', main_color);
	icon_small.format().palette()->set_color('-', 0x0_rgba);

	graphical::software_window win("truwo", {400,400}, graphical::window::flags::resizable);
	auto fg_color = win.surface().format().color(main_color);
	auto bg_color = win.surface().format().color(second_color);

	surface icon({64,64}, pixel_format(pixel_format::type::rgba8888));
	blit(convert( icon_small, icon.format()), icon, rect{icon.size()});
	win.icon(icon);

	auto music = argc > 1
		?  argv[1][0] != '\0' ? std::optional<musical::wav>(argv[1]) : std::nullopt
		: std::optional<musical::wav>("./truwo.wav");

	std::atomic<bool> music_playing = false;
	// std::atomic<bool> music_done = false;
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

	std::vector<movement> timers;
	timers.reserve(100); // TODO: hack to avoid invalidation
	auto current_timer = motion::symphony{support::make_range(timers)};
	bool paused = true;

	entities ui{ components<object_interface<i_ui_object, i_graphic, i_interactive>>{} };

	rect button_rect{win.size()/8 + int2{5,5}};
	auto make_control_button = [&]() -> auto&
	{
		auto& button = ui.emplace<plain_button>(fg_color, button_rect);
		return button;
	};
	auto& stop_button = make_control_button();
	auto& down_button = make_control_button();

	focus_vector button_focus_group{
		{&stop_button, &down_button} };

	focus_vector main_focus_group
	{{
		&button_focus_group,
	}};

	auto focus_handler = [&main_focus_group](auto& element)
	{
		main_focus_group.focus_on(&element);
	};

	stop_button.on_press.push_back(focus_handler);
	down_button.on_press.push_back(focus_handler);
	bounds_layout button_layout ({&stop_button, &down_button}, int2::i(5));
	button_layout.update();

	bounds_layout main_layout({&button_layout}, int2::j(15));
	main_layout.update();

	main_layout += int2(10,10);

	auto make_timer_ui = [&ui, &fg_color, &focus_handler]()
	{
		auto [hours_display, minutes_display, seconds_display] =
			ui.make([&](auto& components)
		{
			int2 digit_size{40,100};
			auto digit_spacing = int2::i(5);
			auto make_time_display = [&]() -> auto&
			{
				// oof, template -_-
				auto& display = components.template emplace<digit_display<>>(digit_size, digit_spacing, fg_color);
				return display;
			};
			return std::tie
			(
				make_time_display(),
				make_time_display(),
				make_time_display()
			);
		}).components;

		hours_display.on_press.push_back(focus_handler);
		minutes_display.on_press.push_back(focus_handler);
		seconds_display.on_press.push_back(focus_handler);

		rect separator_rect{{13,100}};
		bounds_layout timer_layout(
			{
				&hours_display,
				&ui.emplace<digit_bitmap>(digit[10], fg_color, separator_rect),
				&minutes_display,
				&ui.emplace<digit_bitmap>(digit[10], fg_color, separator_rect),
				&seconds_display
			},
			int2::i(5)
		);
		timer_layout.update();

		return std::tuple_cat(
			std::tie(hours_display, minutes_display, seconds_display),
			std::tuple{
				focus_vector{ {&hours_display, &minutes_display, &seconds_display} },
				std::move(timer_layout)
			}
		);
	};
	std::vector<decltype(make_timer_ui())> timer_displays;
	timer_displays.reserve(timers.capacity()); // TODO: hack to avoid invalidation

	auto reset_current_timer = [&]()
	{
		music_playing = false;
		init.graphics.screensaver.release_one();
		device = nullptr;
		current_timer.reset();
		paused = true;
	};

	stop_button.on_click.push_back([&](auto&)
	{
		if(music_playing && current_timer.done())
		{
			reset_current_timer();
		}
		else
		{
			paused = true;
		}
	});

	down_button.on_click.push_back([&](auto&)
	{
		paused = false;
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

	bool done = false;
	std::chrono::steady_clock::time_point current_time;
	while(!done)
	{
		const auto new_time = std::chrono::steady_clock::now();
		const auto time_delta = new_time - current_time;
		current_time = new_time;

		using namespace interactive;
		while(auto event = next_event())
		{
			std::visit(support::overloaded{
				[&done](quit_request) { done = true; },
				[&win](window_size_changed)
				{
					win.update_surface();
				},
				[&main_focus_group](const mouse_down&)
				{
					main_focus_group.drop_focus();
				},
				[&main_focus_group, &make_timer_ui, &timer_displays, &timers, &main_layout, &current_timer](const key_pressed& e)
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
					else if(e.data.keycode == keycode::n)
					{
						auto& [h,m,s, focus, layout] = timer_displays.emplace_back(make_timer_ui());

						auto last_total_duration = empty(timers) ? 0ms
							: timers.back().total;
						auto& timer = timers.emplace_back(movement{last_total_duration});
						current_timer = motion::symphony{support::make_range(timers)};

						main_focus_group.drop_focus();
						auto elements = std::move(main_focus_group).extract();
						elements.push_back(&focus);
						main_focus_group = focus_vector(std::move(elements));

						main_layout.elements.push_back(&layout);
						main_layout.update();

						h.on_input.push_back([&timer](auto&&, int old_value, int new_value)
						{
							using namespace std::chrono;
							auto offset = hours(new_value) - hours(old_value);
							timer.reset();
							timer.total = timer.total - timer.elapsed + offset;
						});
						m.on_input.push_back([&timer](auto&&, int old_value, int new_value)
						{
							using namespace std::chrono;
							auto new_minutes = minutes(new_value);
							if(new_minutes >= hours(1))
								new_minutes = hours(1) - minutes(1);
							auto offset = new_minutes - minutes(old_value);
							timer.reset();
							timer.total = timer.total - timer.elapsed + offset;
						});
						s.on_input.push_back([&timer](auto&&, int old_value, int new_value)
						{
							using namespace std::chrono;
							auto new_seconds = seconds(new_value);
							if(new_seconds >= minutes(1))
								new_seconds = minutes(1) - seconds(1);
							auto offset = new_seconds - seconds(old_value);
							timer.reset();
							timer.total = timer.total - timer.elapsed + offset;
						});

					}
				},
				[](auto) { }
			}, *event);

			for(auto&& interactive : ui.get<i_interactive*>())
				interactive->update(*event);
		}

		for
		(
			auto [timer, display] = std::tuple
			{
				timers.begin(),
				timer_displays.begin()
			};
			timer < timers.end();
			++timer, ++display
		)
		{
			using std::get;
			auto duration = timer->total - timer->elapsed;
			get<0>(*display).set(extract_duration<std::chrono::hours>(duration).count());
			get<1>(*display).set(extract_duration<std::chrono::minutes>(duration).count());
			get<2>(*display).set(extract_duration<std::chrono::seconds>(duration).count());
		}

		fill(win.surface(), bg_color);

		for(auto&& graphic : ui.get<i_graphic*>())
			graphic->draw(win.surface());
		win.update();

		down_button.enable(!music_playing && paused);
		for(auto&& timer_display : timer_displays)
		{
			using std::get;
			get<0>(timer_display).enable(!music_playing && paused);
			get<1>(timer_display).enable(!music_playing && paused);
			get<2>(timer_display).enable(!music_playing && paused);
		}

		if(stop_button_hold.check())
			reset_current_timer();

		if(!music_playing)
		{
			if(timers.size() != 0 && current_timer.done())
			{
				reset_current_timer();
			}
		}

		if(!paused)
		{
			auto result = current_timer.advance(time_delta);
			auto updated_displays = support::map_range(result.updated, timer_displays, timers.begin());

			for
			(
				auto [timer, display] = std::tuple
				{
					result.updated.begin(),
					updated_displays.begin()
				};
				timer < result.updated.end();
				++timer, ++display
			)
			{
				if(timer->done())
				{
					if(!music_playing)
					{
						music_playing = true;
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
				}
			}
			if(result.done)
			{
				paused = true;
				// TODO:
			}
		}

		const auto next_frame_time = current_time + frametime;
		// const auto next_frame_time = paused
		// 	? current_time + frametime
		// 	: min(current_timer.target_time_point(), current_time + frametime);
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
