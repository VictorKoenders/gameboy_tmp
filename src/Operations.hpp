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
		auto result = from + to;

		state.cycles += 8;

		auto flags = Flags::Empty;
		if (result == 0) {
			flags |= Flags::Zero;
		}
		auto carrybits = from ^ to ^ result;
		if ((carrybits & 0x100) != 0) {
			flags |= Flags::Carry;
		}
		if ((carrybits & 0x10) != 0) {
			flags |= Flags::HalfCarry;
		}
		state.cpu.flags = flags;

		TTo::set(state, result);
	}
};

template <typename TFrom>
class Sub
{
public:
	static void execute(State &state)
	{
		auto from = TFrom::get(state);
		auto to = state.cpu.A;
		auto result = to - from;

		state.cycles += 8;

		auto flags = Flags::Subtract;
		if (result == 0) {
			flags |= Flags::Zero;
		}
		auto carrybits = state.cpu.A ^ from ^ result;
		if ((carrybits & 0x100) != 0) {
			flags |= Flags::Carry;
		}
		if ((carrybits & 0x10) != 0) {
			flags |= Flags::HalfCarry;
		}
		state.cpu.flags = flags;

		state.cpu.A = result;
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
		auto flags = state.cpu.flags & Flags::Carry;
		if ((val + 1) == 0) {
			flags |= Flags::Zero;
		}
		if ((val & 0b111) == 0b111) {
			flags |= Flags::HalfCarry;
		}
		state.cpu.flags = flags;

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
		auto flags = (state.cpu.flags & Flags::Carry) | Flags::Subtract;
		if ((val - 1) == 0) {
			flags |= Flags::Zero;
		}
		if ((val & 0b1000) == 0b1000) {
			flags |= Flags::HalfCarry;
		}
		state.cpu.flags = flags;


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

template <typename TTo, typename TFrom>
class LoadIntoMemory
{
public:
	static void execute(State &state)
	{
		auto from = TFrom::get(state);
		auto address = 0xFF00 + static_cast<int8_t>(TTo::get(state));

		state.cycles += 12;

		state.set_memory_byte(address, from);
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

template <int N, typename TField>
class Bit
{
public:
	static void execute(State& state) {
		uint8_t value = TField::get(state);
		Flags flags = (state.cpu.flags & Flags::Carry) | Flags::HalfCarry;
		if ((value & (0x1 << N)) == 0) {
			flags |= Flags::Zero;
		}

		state.cycles += 8;

		state.cpu.flags = flags;
	}
};

template <typename TField>
class Push
{
public:
	static void execute(State& state) {
		int16_t value = TField::get(state);
		state.set_memory_byte(state.cpu.sp, static_cast<uint8_t>(value & 0xFF));
		state.set_memory_byte(state.cpu.sp - 1, static_cast<uint8_t>(value >> 8));
		state.cycles += 16;
		state.cpu.sp -= 2;
	}
};

template <typename TField>
class Pop
{
public:
	static void execute(State& state) {
		auto high = state.get_memory_byte(state.cpu.sp + 1);
		auto low = state.get_memory_byte(state.cpu.sp + 2);
		state.cpu.sp += 2;
		TField::set(state, (static_cast<uint16_t>(high) << 8) | static_cast<uint16_t>(low));

		state.cycles += 12;
	}
};

template <typename TField>
class Call
{
public:
	static void execute(State& state) {
		int16_t value = TField::get(state);
		auto pc = state.cpu.pc;
		state.cpu.pc = value;

		state.set_memory_byte(state.cpu.sp, static_cast<uint8_t>(pc & 0xFF));
		state.set_memory_byte(state.cpu.sp - 1, static_cast<uint8_t>(pc >> 8));

		state.cycles += 24;
		state.cpu.sp -= 2;
	}
};

template <typename TCondition>
class Ret
{
public:
	static void execute(State& state) {
		if (TCondition::is_true(state)) {
			auto high = state.get_memory_byte(state.cpu.sp + 1);
			auto low = state.get_memory_byte(state.cpu.sp + 2);
			state.cpu.sp += 2;
			state.cpu.pc = (static_cast<uint16_t>(high) << 8) | static_cast<uint16_t>(low);
			state.cycles += 8;

		}
	}
};

template <typename TField>
class RotateLeft
{
public:
	static void execute(State& state) {
		int8_t value = TField::get(state);
		auto new_carry_set = (value & 0b10000000) > 0;
		value = value << 1;
		if ((state.cpu.flags & Flags::Carry) == Flags::Carry) {
			value += 1;
		}

		Flags flags = Flags::Empty;

		if (new_carry_set) {
			flags |= Flags::Carry;
		}
		if (value == 0) {
			flags |= Flags::Zero;
		}
		state.cpu.flags = flags;
		TField::set(state, value);
		state.cycles += 8;
	}
};


