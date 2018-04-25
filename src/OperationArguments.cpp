#include "OperationArguments.hpp"

int16_t HL::get(const State& state)
{
	return state.cpu.HL;
}

void HL::set(State &state, int16_t value)
{
	state.cpu.HL = value;
}

int8_t C::get(const State &state)
{
	return state.cpu.C;
}

void C::set(State &state, int8_t value)
{
	state.cpu.C = value;
}

int8_t D8::get(State &state)
{
	return state.consume_byte();
}

void D8::set(State &state, int8_t value)
{
	std::cout << "Error; cannot set a D8 value" << std::endl;
	state.is_error = true;
}

int16_t BC::get(const State &state)
{
	return state.cpu.BC;
}

void BC::set(State &state, int16_t value)
{
	state.cpu.BC = value;
}
