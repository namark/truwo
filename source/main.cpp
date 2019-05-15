#include <atomic>
#include <cstdio>
#include <cerrno>

#include "simple.hpp"
#include "plain_button.h"
#include "layout.h"
#include "digits.h"
#include "ui_factory.hpp"
#include "utils.hpp"

int main(int argc, const char** argv) try
{
	initializer init;
	ui_factory ui;

	auto music = argc >= 2 ? std::optional<musical::wav>(argv[1]) : std::nullopt;

	graphical::software_window win("melno", {400,200});
	auto fg_color = win.surface().format().color(main_color);
	auto bg_color = win.surface().format().color(0x0_rgb);

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

	auto& up_button = ui.make<plain_button>(fg_color, rect{win.size()/10});
	up_button.on_click.push_back([&](button& button)
	{
		if(!music || music_playing)
			return;
		music_playing = true;
		button.disable();
		device = std::make_unique<music_device>(
			musical::basic_device_parameters{music->obtained()},
			player
		);
		device->play();
	});

	auto& stop_button = ui.make<plain_button>(fg_color, anchored_rect{win.size()/10, int2::zero(), float2::one(0.5)});
	stop_button.on_click.push_back([&](button&)
	{
		if(music_playing)
		{
			music_playing = false;
			up_button.enable();
			device = nullptr;
		}
	});

	auto& down_button = ui.make<plain_button>(fg_color, anchored_rect{win.size()/10, int2::zero(), float2::one(0.5)});
	down_button.on_click.push_back([&](button&)
	{
	});

	bounds_layout ({&up_button, &stop_button, &down_button}, int2::j(5)).update();

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
