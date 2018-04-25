#pragma once
#include "State.hpp"
#include <iostream>

class HL
{
public:
	static int16_t get(const State &state);
	static void set(State &state, int16_t value);
};

class C
{
public:
	static int8_t get(const State &state);
	static void set(State &state, int8_t value);
};

class D8
{
public:
	static int8_t get(State &state);
	static void set(State &state, int8_t value);
};

class BC
{
public:
	static int16_t get(const State &state);
	static void set(State &state, int16_t value);
};

