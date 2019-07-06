#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstddef>
#include <limits>
#include <type_traits>
#include <iterator>

#include "simple/support/algorithm.hpp"
#include "simple.hpp"

// this was really hard :/
template <typename Range>
class loop
{
	public:
	using iterator = decltype(std::begin(std::declval<Range>()));
	using difference_type = typename std::iterator_traits<iterator>::difference_type;
	using value_type = typename std::iterator_traits<iterator>::value_type;
	using pointer = typename std::iterator_traits<iterator>::pointer;
	using reference = typename std::iterator_traits<iterator>::reference;
	using iterator_category = typename std::iterator_traits<iterator>::iterator_category;

	constexpr loop(Range range, iterator current) :
		range(range), current(current)
	{ }

	constexpr loop(Range range) :
		range(range), current(std::begin(range))
	{ }

	constexpr auto operator*() const
	{ return *current; }
	constexpr auto operator->() const
	{ return current.operator->(); }
	constexpr auto operator*()
	{ return *current; }
	constexpr auto operator->()
	{ return current.operator->(); }

	constexpr loop& operator++()
	{
		++current;
		if(current == std::end(range))
			current = std::begin(range);
		return *this;
	}

	constexpr loop operator++(int)
	{
		loop ret = loop(range, current++);
		if(current == std::end(range))
			current = std::begin(range);
		return ret;
	}

	constexpr loop& operator+=(difference_type d)
	{
		const iterator begin = std::begin(range);
		const iterator end = std::end(range);
		const difference_type size = std::distance(begin, end);
		const difference_type current_index = std::distance(begin, current);
		current = begin + simple::support::wrap((current_index + d) % size, size);
		return *this;
	}
	constexpr loop& operator-=(difference_type d)
	{
		return *this += -d;
	}

	constexpr friend loop operator+(const loop& one, difference_type d)
	{
		return loop(one) += d;
	}
	constexpr friend loop operator+(difference_type d, const loop& one)
	{
		return loop(one) += d;
	}

	constexpr friend loop operator-(const loop& one, difference_type d)
	{
		return loop(one) -= d;
	}
	constexpr friend loop operator-(difference_type d, const loop& one)
	{
		return loop(one) -= d;
	}

	constexpr friend bool operator==(const loop& one, const loop& other)
	{
		return one.range == other.range && one.current == other.current;
	}
	constexpr friend bool operator!=(const loop& one, const loop& other)
	{
		return !(one == other);
	}

	constexpr operator iterator()
	{
		return current;
	}

	constexpr void reset()
	{
		current = std::begin(range);
	}

	private:
	Range range;
	iterator current;
};

class random_access_istream_iterator // more or less
{
	// TODO:
};

template <typename Range>
constexpr auto center(const Range& area)
{
	return (area.upper() - area.lower())/2;
}

template <typename Object, typename Area>
constexpr auto center(const Object& object, const Area& area)
{
	return center(area) - center(object);
}

void outline(const graphical::surface&, graphical::color, range2D);
void lowlight(const graphical::surface&, graphical::color, range2D, int2 dim = int2(2,1));
void outline_lowlight(const graphical::surface&, graphical::color, range2D, int2 dim = int2(2,2));

template <typename ExtractedDuration, typename Duration>
ExtractedDuration extract_duration(Duration& duration)
{
	auto extracted = std::chrono::duration_cast<ExtractedDuration>(duration);
	duration -= extracted;
	return extracted;
}

template <typename... SplitDuration, typename Duration>
std::tuple<SplitDuration...> split_duration(Duration duration)
{
	return { extract_duration<SplitDuration>(duration)... };
};

#endif /* end of include guard */
