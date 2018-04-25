#include "OperationArguments.hpp"

uint16_t HL::get(const State& state)
{
	return state.cpu.HL;
}

void HL::set(State &state, uint16_t value)
{
	state.cpu.HL = value;
}

uint8_t C::get(const State &state)
{
	return state.cpu.C;
}

void C::set(State &state, uint8_t value)
{
	state.cpu.C = value;
}

uint8_t D::get(const State &state)
{
	return state.cpu.D;
}

void D::set(State &state, uint8_t value)
{
	state.cpu.D = value;
}

uint8_t E::get(const State &state)
{
	return state.cpu.E;
}

void E::set(State &state, uint8_t value)
{
	state.cpu.E = value;
}

uint8_t H::get(const State &state)
{
	return state.cpu.H;
}

void H::set(State &state, uint8_t value)
{
	state.cpu.H = value;
}

uint8_t L::get(const State &state)
{
	return state.cpu.L;
}

void L::set(State &state, uint8_t value)
{
	state.cpu.L = value;
}

uint8_t D8::get(State &state)
{
	auto value = state.get_memory_byte(state.cpu.pc);
	state.cpu.pc++;
	return value;
}

void D8::set(State &state, uint8_t value)
{
	state.panic("Cannot write to D8 address");
}

uint16_t BC::get(const State &state)
{
	return state.cpu.BC;
}

void BC::set(State &state, uint16_t value)
{
	state.cpu.BC = value;
}

uint16_t D16::get(State & state)
{
	auto first = state.get_memory_byte(state.cpu.pc);
	state.cpu.pc++;
	auto second = state.get_memory_byte(state.cpu.pc);
	state.cpu.pc++;
	return (static_cast<uint16_t>(second) << 8) | static_cast<uint16_t>(first);
}

void D16::set(State & state, uint16_t value)
{
	state.panic("Cannot write to D16 address");
}

uint8_t A::get(const State & state)
{
	return state.cpu.A;
}

void A::set(State & state, uint8_t value)
{
	state.cpu.A = value;
}

uint8_t B::get(const State & state)
{
	return state.cpu.B;
}

void B::set(State & state, uint8_t value)
{
	state.cpu.B = value;
}

uint8_t PtrBC::get(const State & state)
{
	return state.get_memory_byte(state.cpu.BC);
}

void PtrBC::set(State & state, uint8_t value)
{
	state.set_memory_byte(state.cpu.BC, value);
}

uint8_t PtrHL::get(const State & state)
{
	return state.get_memory_byte(state.cpu.HL);
}

void PtrHL::set(State & state, uint8_t value)
{
	state.set_memory_byte(state.cpu.HL, value);
}

bool Z::is_true(const State & state)
{
	return (state.cpu.flags & Flags::Zero) == Flags::Zero;
}

int16_t SPR8::get(State & state)
{
	int8_t value = state.get_memory_byte(state.cpu.pc);
	state.cpu.pc++;
	return state.cpu.sp + static_cast<int16_t>(value);
}

void SPR8::set(State & state, int16_t)
{
	state.panic("Can not set SP + R8");
}
