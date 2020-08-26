#ifndef UI_FACTORY_HPP
#define UI_FACTORY_HPP

#include <vector>
#include <memory>

#include "simple.hpp"
#include "implementation.h"

class ui_factory
{
	std::vector<std::unique_ptr<i_ui_object>> elements;
	std::vector<i_graphic*> _graphics;
	std::vector<i_interactive*> _interactives;

	public:

	template <typename Element, typename ...Args>
	Element& make(Args&&... args)
	{
		auto element = std::make_unique<Element>(std::forward<Args>(args)...);
		Element& raw = *element.get();
		if constexpr (std::is_base_of_v<i_graphic, Element>)
			_graphics.push_back(&raw);
		if constexpr (std::is_base_of_v<i_interactive, Element>)
			_interactives.push_back(&raw);

		elements.push_back(std::move(element)); // assuming raw pointer will not change... that's safe right?
		return raw;
	}

	const std::vector<i_graphic*>& graphics() const noexcept
	{ return _graphics; }
	const std::vector<i_interactive*>& interactives() const noexcept
	{ return _interactives; }
};


#endif /* end of include guard */
