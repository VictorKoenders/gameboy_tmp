#pragma once
#include "State.hpp"
#include <iostream>

class HL
{
public:
	static uint16_t get(const State &state);
	static void set(State &state, uint16_t value);
};

class A
{
public:
	static uint8_t get(const State &state);
	static void set(State &state, uint8_t value);
};

class B
{
public:
	static uint8_t get(const State &state);
	static void set(State &state, uint8_t value);
};

class C
{
public:
	static uint8_t get(const State &state);
	static void set(State &state, uint8_t value);
};

class D
{
public:
	static uint8_t get(const State &state);
	static void set(State &state, uint8_t value);
};

class E
{
public:
	static uint8_t get(const State &state);
	static void set(State &state, uint8_t value);
};

class H
{
public:
	static uint8_t get(const State &state);
	static void set(State &state, uint8_t value);
};

class L
{
public:
	static uint8_t get(const State &state);
	static void set(State &state, uint8_t value);
};

class D8
{
public:
	static uint8_t get(State &state);
	static void set(State &state, uint8_t value);
};

class D16
{
public:
	static uint16_t get(State &state);
	static void set(State &state, uint16_t value);
};

class SPR8
{
public:
	static int16_t get(State &state);
	static void set(State &state, int16_t value);
};

class BC
{
public:
	static uint16_t get(const State &state);
	static void set(State &state, uint16_t value);
};

class DE
{
public:
	static uint16_t get(const State &state);
	static void set(State &state, uint16_t value);
};

class SP
{
public:
	static uint16_t get(const State &state);
	static void set(State &state, uint16_t value);
};

class PtrC
{
public:
	static uint8_t get(State &state);
	static void set(State &state, uint8_t value);
};

class PtrBC
{
public:
	static uint8_t get(State &state);
	static void set(State &state, uint8_t value);
};

class PtrDE
{
public:
	static uint8_t get(State &state);
	static void set(State &state, uint8_t value);
};

class PtrHL
{
public:
	static uint8_t get(State &state);
	static void set(State &state, uint8_t value);
};

class PtrD8
{
public:
	static uint8_t get(State &state);
	static void set(State &state, uint8_t value);
};

class PtrD16
{
public:
	static uint8_t get(State &state);
	static void set(State &state, uint8_t value);
};

class PtrHLDecr
{
public:
	static uint8_t get(State &state);
	static void set(State &state, uint8_t value);
};

class PtrHLIncr
{
public:
	static uint8_t get(State &state);
	static void set(State &state, uint8_t value);
};


class Z
{
public:
	static bool is_true(const State &state);
};

class NZ
{
public:
	static bool is_true(const State &state);
};

class Always
{
public:
	static bool is_true(const State) { return true; }
};
