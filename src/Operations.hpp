#pragma once
#include "State.hpp"

template <typename TTo, typename TFrom>
class Add
{
public:
	static void operation(State &state)
	{
		auto from = TFrom::get(state);
		auto to = TTo::get(state);
		TTo::set(state, to + from);
	}
};

template <typename TTo, typename TFrom>
class Load
{
public:
	static void operation(State &state)
	{
		auto from = TFrom::get(state);
		TTo::set(state, from);
	}
};

