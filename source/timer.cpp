#include "timer.h"

timer::timer(clock::duration duration, bool autostart) noexcept :
	set(duration),
	remaining(set),
	target(clock::now() + set),
	_paused(!autostart)
{}

bool timer::check() noexcept
{
	if(_paused)
		return false;

	remaining = target - clock::now();
	if(target <= clock::now())
	{
		return true;
	}
	return false;
}

auto timer::duration() const noexcept
	-> clock::duration
{
	return set;
}

auto timer::remaining_duration() const noexcept
	-> clock::duration
{
	return remaining;
}

auto timer::target_time_point() const noexcept
	-> clock::time_point
{
	if(_paused)
		return clock::now() + remaining;
	else
		return target;
}

void timer::pause(bool value) noexcept
{
	if(value)
		pause();
	else
		resume();
}

void timer::pause() noexcept
{
	_paused = true;
}

void timer::resume() noexcept
{
	_paused = false;
	target = clock::now() + remaining;
}

bool timer::paused() const noexcept
{
	return _paused;
}

