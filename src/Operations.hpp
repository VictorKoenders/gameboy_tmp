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

template <typename TFrom>
class SubCarry
{
public:
	static void execute(State &state)
	{
		state.panic("SBC not implemented");
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

		state.cpu.flags = result == 0 ? Flags::Zero : Flags::Empty;
		state.cpu.A = result;
	}
};

template <typename TField>
class And
{
public:
	static void execute(State &state)
	{
		state.panic("AND not implemented");
	}
};

template <typename TField>
class Or
{
public:
	static void execute(State &state)
	{
		state.panic("Or not implemented");
	}
};


template <typename TField>
class Inc
{
public:
	static void execute(State &state)
	{
		auto val = TField::get(state);
		
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
			state.cycles += 4;
			state.cpu.pc += val;
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
			state.cycles += 4;
			state.cpu.pc += val;
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

		state.cpu.flags = flags;
	}
};

template <int N, typename TField>
class ClearBit
{
public:
	static void execute(State& state) {
		state.panic("ClearBit not implemented");
	}
};

template <int N, typename TField>
class SetBit
{
public:
	static void execute(State& state) {
		state.panic("SetBit not implemented");
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
	}
};

template <typename TCondition, typename TField>
class Call
{
public:
	static void execute(State& state) {
		int16_t value = TField::get(state);
		if (TCondition::is_true(state)) {
			auto pc = state.cpu.pc;
			state.cpu.pc = value;

			state.set_memory_byte(state.cpu.sp, static_cast<uint8_t>(pc & 0xFF));
			state.set_memory_byte(state.cpu.sp - 1, static_cast<uint8_t>(pc >> 8));

			state.cycles += 12;
			state.cpu.sp -= 2;
		}
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
			state.cycles += 12;
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
	}
};


template <typename TField>
class RotateLeftCarry
{
public:
	static void execute(State& state) {
		state.panic("RotateLeftCarry not implemented");
	}
};


template <typename TField>
class RotateRight
{
public:
	static void execute(State& state) {
		state.panic("RotateRight not implemented");
	}
};


template <typename TField>
class RotateRightCarry
{
public:
	static void execute(State& state) {
		state.panic("RotateRightCarry not implemented");
	}
};

template <typename TField>
class ShiftLeft
{
public:
	static void execute(State& state) {
		state.panic("ShiftLeft not implemented");
	}
};

template <typename TField>
class ShiftRight
{
public:
	static void execute(State& state) {
		state.panic("ShiftRight not implemented");
	}
};

template <typename TField>
class ShiftRightClear
{
public:
	static void execute(State& state) {
		state.panic("ShiftRightClear not implemented");
	}
};

template <typename TField>
class Swap
{
public:
	static void execute(State& state) {
		state.panic("Swap not implemented");
	}
};

class PrefixCB {
public:
	static void execute(State& state) {
		state.process_cb();
	}
};

