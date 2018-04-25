#pragma once
#include "State.hpp"

class Nop {
public:
	static void execute(State& state)
	{
	}
};

template <typename TTo, typename TFrom>
class Add
{
public:
	static void execute(State &state)
	{
		auto from = TFrom::get(state);
		auto to = TTo::get(state);

		state.cycles += 8;

		TTo::set(state, to + from);
	}
};

template <typename TTo, typename TFrom>
class AddCarry
{
public:
	static void execute(State &state)
	{
		state.panic("ADC not implemented");
	}
};

template <typename TField>
class Xor
{
public:
	static void execute(State &state)
	{
		auto value = TField::get(state);
		auto result = value ^ state.cpu.A;

		state.cycles += 4;

		state.cpu.flags = result == 0 ? Flags::Zero : Flags::Empty;
		state.cpu.A = result;
	}
};


template <typename TField>
class Inc
{
public:
	static void execute(State &state)
	{
		auto val = TField::get(state);
		
		state.cycles += 8;

		TField::set(state, val + 1);
	}
};

template <typename TField>
class Dec
{
public:
	static void execute(State &state)
	{
		auto val = TField::get(state);

		state.cycles += 4;

		TField::set(state, val - 1);
	}
};

template <typename TTo, typename TFrom>
class Load
{
public:
	static void execute(State &state)
	{
		auto from = TFrom::get(state);

		// TODO: These cycles are weird
		// If the instruction loads 2 bytes, this instruction takes 12 cycles
		// If the instruction loads or writes 1 byte, this instruction takes 8 cycles
		// If the intruction is between CPU registers, then this instruction takes 4 cycles

		// For now we'll just use 8 cycles
		state.cycles += 8;

		TTo::set(state, from);
	}
};

template <typename TCondition, typename TField>
class JumpRelative
{
public:
	static void execute(State &state)
	{
		int8_t val = static_cast<int8_t>(TField::get(state));
		if (TCondition::is_true(state)) {
			state.cycles += 12;
			state.cpu.pc += val;
		}
		else {
			state.cycles += 8;
		}
	}
};

template <typename TCondition, typename TField>
class Jump
{
public:
	static void execute(State &state)
	{
		// TODO: This is not true for the 0xE9 JP (HL) instruction, that should be 4 cycles anyway

		auto val = TField::get(state);
		if (TCondition::is_true(state)) {
			state.cycles += 16;
			state.cpu.pc += val;
		}
		else {
			state.cycles += 12;
		}
	}
};

template <typename TField>
class Compare
{
public:
	static void execute(State &state)
	{
		auto val = TField::get(state);
		auto flags = Flags::Subtract;
		if (state.cpu.A == val) {
			flags |= Flags::Zero;
		}
		if (((state.cpu.A & 0xF) - (val * 0xF)) < 0) {
			flags |= Flags::HalfCarry;
		}
		if (state.cpu.A < val) {
			flags |= Flags::Carry;
		}

		// TODO: If this is 0xBE CP (HL) or 0xFE CP d8 then this instruction costs 8 cycles instead
		state.cycles += 4;

		state.cpu.flags = flags;
	}
};


