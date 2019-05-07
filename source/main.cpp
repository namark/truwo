#include <iostream>
#include <atomic>
#include <cstdio>
#include <cerrno>

#include "simple.hpp"
#include "plain_button.h"
#include "layout.h"
#include "digits.h"

int main(int argc, const char** argv) try
{
	initializer init;

	auto music = argc >= 2 ? std::optional<musical::wav>(argv[1]) : std::nullopt;

	graphical::software_window win("melno", {400,200});
	auto fg_color = win.surface().format().color(main_color);
	auto bg_color = win.surface().format().color(0x0_rgb);

	std::atomic<bool> music_playing = false;
	auto player = [&music_playing, &music, i = music->buffer().begin()](auto& device, auto buffer) mutable
	{
		if(!music_playing)
		{
			i = music->buffer().begin();
			std::fill(buffer.begin(), buffer.end(), device.silence());
			return;
		}

		const int remaining = music->buffer().end() - i;
		const int size = std::min<int>(remaining, buffer.size);
		const int extra = buffer.size - size;

		std::copy(i, i + size, buffer.begin());
		std::fill_n(buffer.begin() + size, extra, device.silence());

		i += size;
		if(i == music->buffer().end())
			music_playing = false;
	};
	using music_device = musical::device_with_callback<decltype(player)>;
	std::unique_ptr<music_device> device = nullptr;

	std::vector<std::unique_ptr<button>> buttons;
	buttons.push_back(std::make_unique<plain_button>(fg_color, anchored_rect{win.size()/10, win.size()/2, float2::one(0.5f)}));

	buttons.front()->on_click.push_back([&](button& button)
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

	std::vector<plain_button> boxes;
	for(int i = 0; i < 16; ++i)
		boxes.emplace_back(fg_color, range2D{int2::zero(), int2::one(i%3?20:30)});

	std::vector<bitmap<digit_data::size.x(), digit_data::size.y()>> digs;
		for(int i = 0; i < 10; ++i)
			digs.emplace_back(digit[i], fg_color, range2D{int2::zero(), int2(30,50)});


	std::vector<bounds_layout> layouts;
	for(int i = 0; i < 4; ++i)
	{
		bounds_layout test_layout(int2::i(5));
		test_layout.elements.push_back(&boxes[i*4]);
		test_layout.elements.push_back(&boxes[i*4 + 1]);
		test_layout.elements.push_back(&boxes[i*4 + 2]);
		test_layout.elements.push_back(&boxes[i*4 + 3]);
		test_layout.update();
		layouts.push_back(test_layout);
	};
	bounds_layout digit_layout(int2::i(3));
	for(auto&& dig : digs)
		digit_layout.elements.push_back(&dig);
	digit_layout.update();
	layouts.push_back(digit_layout);

	bounds_layout main_layout(int2::j(5));
	for(auto&& layout : layouts)
		main_layout.elements.push_back(&layout);
	main_layout.update();

	bool done = false;
	while(!done)
	{
		const auto current_time = std::chrono::steady_clock::now();

		while(auto event = interactive::next_event())
		{
			std::visit(support::overloaded{
				[&done](interactive::quit_request)
				{
					done = true;
				},
				[&done](interactive::key_pressed key)
				{
					if(key.data.scancode == interactive::scancode::escape)
						done = true;
				},
				[](auto) { }
			}, *event);

			for(auto&& button : buttons)
				button->update(*event);
			for(auto&& box : boxes)
				box.update(*event);
		}

		if(!music_playing && buttons.front()->current_state() == button::state::disabed)
		{
			device = nullptr;
			buttons.front()->enable();
		}
		fill(win.surface(), bg_color);

		for(auto&& button : buttons)
			button->draw(win.surface());

		for(auto&& box : boxes)
			box.draw(win.surface());

		for(auto&& dig : digs)
			dig.draw(win.surface());

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
