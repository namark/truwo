#ifndef UI_FACTORY_HPP
#define UI_FACTORY_HPP

#include <vector>
#include <memory>

#include "simple/support/tuple_utils.hpp"


template <typename... Interfaces>
class entities;

using simple::support::flatten_t;
using simple::support::flatten_meta_operator;

template <typename T, T Offset, T... Is>
constexpr auto offset_sequence(std::integer_sequence<T,Is...>)
{
	return std::integer_sequence<T, (Is + Offset)...>{};
}

template <typename T, T begin, T size>
constexpr auto make_integer_segment()
{
	return offset_sequence<T, begin>(std::make_integer_sequence<T, size>{});
}

template <typename Base, typename... Interfaces>
// TODO: enable if is_abstract<Base> && has_virtual_destructor<Base>
// and maybe is_abstract<Interfaces>, but not sure
struct object_interface
{
	static constexpr size_t type_count = 1 + sizeof...(Interfaces);
};

// TODO: parameterise std::unique_ptr and std::vector templates?? do I need that within a project or between projects, cause in a latter case can get away with using declaration in a configuration header
template <typename... Types>
class components
{
	public:

	using types = std::tuple<Types...>;

	template <typename Element, typename ...Args>
	Element& emplace(Args&&... args)
	{
		using simple::support::find_meta_t;
		using simple::support::bind_meta;
		using simple::support::tuple_car;
		using simple::support::tuple_car_t;
		using simple::support::tuple_tie_cdr;
		using simple::support::tie_subtuple;
		using simple::support::is_template_instance_v;

		using found = find_meta_t<bind_meta<is_component, size_constant<0>, Element>, types>;
		static_assert(found::value != std::tuple_size_v<types>, "Element type not found in known component list.");
		if constexpr (is_template_instance_v<object_interface, typename found::type>)
		{
			auto object_vectors = tie_subtuple(vectors,
				make_integer_segment<size_t,
					typename found::functor::binding{},
					found::type::type_count
				>()
			);

			// get the tuple of the pointer interface and do this
			auto& elements = tuple_car(object_vectors);

			auto element = std::make_unique<Element>(std::forward<Args>(args)...);
			Element& raw = *element.get();

			add_interface(raw, tuple_tie_cdr(object_vectors));

			elements.emplace_back(std::move(element)); // assuming raw pointer will not change... that's safe right?
			return raw;
		}
		else
		{
			return std::get<found::value>().emplace_back(std::forward<Args>(args)...);
		}
	}

	// TODO: return a range
	template <typename Component>
	const auto& get() const noexcept
	{
		using simple::support::find_v;
		return std::get<find_v<Component, flat_types>>(vectors);
	}

	private:

	template <typename T>
	struct flatten_object_interface : flatten_meta_operator<T> {};
	template <typename Base, typename... Interfaces>
	struct flatten_object_interface<object_interface<Base, Interfaces...>>
	{
		using type = std::tuple<std::unique_ptr<Base>, Interfaces*...>;
	};

	template <size_t i> using size_constant = std::integral_constant<size_t, i>;

	template <typename flat_index, typename T, typename Component>
	struct is_component : std::is_same<T,Component>
	{
		using binding = size_constant<flat_index{} + !is_component::value>;
	};

	template <typename flat_index, typename T, typename Base, typename... Interfaces>
	struct is_component<flat_index, T, object_interface<Base,Interfaces...>>
	{
		static constexpr bool value = std::is_base_of_v<Base, T>;
		static constexpr size_t increment =
			[](bool value)
			{
				return value
					? 0
					: object_interface<Base,Interfaces...>::type_count
				;
			}
			(is_component::value);
		using binding = size_constant<flat_index{} + increment>;
	};

	using flat_types = flatten_t<types,flatten_object_interface>;

	template <typename Tuple, bool ref = false>
	struct container { };

	template <typename... Ts>
	struct container<std::tuple<Ts...>, false>
	{
		using type = std::tuple<std::vector<Ts>...>;
	};

	template <typename... Ts>
	struct container<std::tuple<Ts...>, true>
	{
		using type = std::tuple<std::vector<Ts>&...>;
	};

	template <typename Tuple>
	using container_t = typename container<Tuple>::type;

	template <typename... Ts>
	using container_ref_t = typename container<std::tuple<Ts...>, true>::type;

	template <typename El>
	void add_interface(El&, std::tuple<>){}

	template <typename El, typename In, typename... Rest>
	// NOTE: the shorthand can't deduce the In :/
	// void add_interface(El& element, container_ref_t<In*, Rest...> interfaces)
	void add_interface(El& element, std::tuple<std::vector<In*>&, std::vector<Rest>&...> interfaces)
	{
		using simple::support::tuple_car;
		using simple::support::tuple_tie_cdr;
		if constexpr (std::is_base_of_v<In, El>)
			tuple_car(interfaces).push_back(&element);
		add_interface(element,tuple_tie_cdr(interfaces));
	}

	container_t<flat_types> vectors;

	friend class entities<Types...>;
};

template <typename... Components>
class entities
{
	public:

	using entity_id_t = unsigned;

	entities(components<Components...> components) :
		_components(std::move(components))
	{
	}

	template <typename T>
	struct entity
	{
		public:
		T components;
		const entity_id_t entity_id;

		private:
		entity(T&& components, const entity_id_t& entity_id) :
			components(std::forward<T>(components)),
			entity_id(entity_id)
		{}
		friend class entities;
	};

	// TODO: getter of components based on entity ids

	// TODO; eplace specific components to replace the make function below,
	// component type1, args1
	// component_type2, args2
	// ...
	// component_typeN, argsN
	//
	// maybe use a component helper template, so it looks like this:
	// entities.emplace
	// (
	//   component<Type1>(args1...),
	//   component<Type2>(args2...),
	//   component<Type3>(args3...),
	//   component<Type4>(args4...),
	// );
	// can also sfinae on this helper template.
	//
	// return a predictable entity object with references to all components and the id

	template <typename Function>
	entity<std::invoke_result_t<
		Function, components<Components...>&>>
	make(Function&& function)
	{
		using simple::support::transform;
		auto size = transform([](const auto& x) {return x.size(); },
			_components.vectors);

		auto&& product = std::invoke(
			std::forward<Function>(function),
			_components);

		entity_ids.push_back(get_id());

		transform([](auto& entity_size, auto size, const auto& interface)
		{
			entity_size.push_back(interface.size() - size);
		}, entity_sizes, size, _components.vectors);

		return
		{
			std::forward<decltype(product)>(product),
			entity_ids.back()
		};
	};

	template <typename Element, typename ...Args>
	Element& emplace(Args&&... args)
	{
		return make([&](auto& components) -> auto&
		{
			return
				components.template emplace<Element>
					(std::forward<Args>(args)...);
		}).components;
	}

	bool erase(entity_id_t id)
	{
		auto found = lower_bound
		(
			begin(entity_ids),
			end(entity_ids),
			id
		);

		if(found != end(entity_ids))
		{
			using simple::support::transform;
			transform([&](auto& components, auto& sizes)
			{
				auto size_begin = begin(entity_sizes);
				auto found_size = size_begin +
					(found - begin(entity_ids));

				auto components_begin = begin(components) +
					accumulate(size_begin, found_size, 0);
				components.erase
				(
					components_begin,
					components_begin + *found_size
				);


				sizes.erase(found_size);
			}, _components.components, entity_sizes);

			entity_ids.erase(found);
			return true;
		}

		return false;
	}

	template <typename Interface>
	decltype(auto) get() const noexcept
	{ return _components.template get<Interface>(); }

	private:

	using entity_size_t = unsigned short;

	entity_id_t get_id()
	{
		if(empty(entity_ids))
			return 0;
		return entity_ids.back() + 1; //TODO: what if we run out?
		// but it'll take like two hundred bajillion years D':
		// yes, what will this program do after two hundred bajillion years of runtime?
		// reuse ids? do I smell std::rotate? =)
	}

	using components_t = components<Components...>;
	components_t _components;
	std::vector<entity_id_t> entity_ids;
	std::array<
		std::vector<entity_size_t>,
		std::tuple_size_v<typename components_t::flat_types>
	> entity_sizes;
};


#endif /* end of include guard */
