#include <chrono>

class timer
{
	public:
	using clock = std::chrono::steady_clock;

	timer(clock::duration duration, bool autostart = false) noexcept;
	bool check() noexcept;
	void pause(bool) noexcept;
	void pause() noexcept;
	void resume() noexcept;
	bool paused() const noexcept;

	clock::duration duration() const noexcept;
	clock::duration remaining_duration() const noexcept;
	clock::time_point target_time_point() const noexcept;

	private:
	clock::duration set;
	clock::duration remaining;
	clock::time_point target;
	bool _paused;
};
