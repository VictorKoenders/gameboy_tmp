#include "OperationArguments.hpp"
#include <iomanip>

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
	if (false) {
		std::cout << "    [0x" << std::setw(4) << std::uppercase << std::hex << std::setfill('0') << static_cast<uint32_t>(state.cpu.pc) << "]: "
			<< static_cast<uint32_t>(value) << std::endl;
	}
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

uint16_t DE::get(const State &state)
{
	return state.cpu.DE;
}

void DE::set(State &state, uint16_t value)
{
	state.cpu.DE = value;
}

uint16_t D16::get(State & state)
{
	auto first = state.get_memory_byte(state.cpu.pc);
	auto pc = state.cpu.pc;
	state.cpu.pc++;
	auto second = state.get_memory_byte(state.cpu.pc);
	state.cpu.pc++;
	uint16_t value = (static_cast<uint16_t>(second) << 8) | static_cast<uint16_t>(first);
	if (false) {
		std::cout << "    [0x" << std::setw(4) << std::uppercase << std::hex << std::setfill('0') << static_cast<uint32_t>(pc) << "]: "
			<< static_cast<uint32_t>(value) << std::endl;
	}
	return value;
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

uint8_t PtrBC::get(State & state)
{
	return state.get_memory_byte(state.cpu.BC);
}

void PtrBC::set(State & state, uint8_t value)
{
	state.set_memory_byte(state.cpu.BC, value);
}

uint8_t PtrDE::get(State & state)
{
	return state.get_memory_byte(state.cpu.DE);
}

void PtrDE::set(State & state, uint8_t value)
{
	state.set_memory_byte(state.cpu.DE, value);
}

uint8_t PtrHL::get(State & state)
{
	return state.get_memory_byte(state.cpu.HL);
}

void PtrHL::set(State & state, uint8_t value)
{
	state.set_memory_byte(state.cpu.HL, value);
}

uint8_t PtrD8::get(State & state)
{
	auto address = 0xFF00 + static_cast<uint16_t>(D8::get(state));
	return state.get_memory_byte(address);
}

void PtrD8::set(State & state, uint8_t value)
{
	auto address = 0xFF00 + static_cast<uint16_t>(D8::get(state));
	state.set_memory_byte(address, value);
}

uint8_t PtrD16::get(State & state)
{
	auto value = D16::get(state);
	return state.get_memory_byte(value);
}

void PtrD16::set(State & state, uint8_t value)
{
	auto address = D16::get(state);
	state.set_memory_byte(address, value);
}


bool Z::is_true(const State & state)
{
	// std::cout << "     Z true: " << ((state.cpu.flags & Flags::Zero) == Flags::Zero ? "yes" : "no") << std::endl;
	return (state.cpu.flags & Flags::Zero) == Flags::Zero;
}

bool NZ::is_true(const State & state)
{
	// std::cout << "    NZ true: " << ((state.cpu.flags & Flags::Zero) == Flags::Empty ? "yes" : "no") << std::endl;
	return (state.cpu.flags & Flags::Zero) == Flags::Empty;
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

uint16_t SP::get(const State & state)
{
	return state.cpu.sp;
}

void SP::set(State & state, uint16_t value)
{
	state.cpu.sp = value;
}

uint8_t PtrHLDecr::get(State & state)
{
	auto value = state.get_memory_byte(state.cpu.HL);
	state.cpu.HL--;
	return value;
}

void PtrHLDecr::set(State & state, uint8_t value)
{
	state.set_memory_byte(state.cpu.HL, value);
	state.cpu.HL--;
}

uint8_t PtrHLIncr::get(State & state)
{
	auto value = state.get_memory_byte(state.cpu.HL);
	state.cpu.HL++;
	return value;
}

void PtrHLIncr::set(State & state, uint8_t value)
{
	state.set_memory_byte(state.cpu.HL, value);
	state.cpu.HL++;
}

uint8_t PtrC::get(State & state)
{
	return state.get_memory_byte(0xFF00 + static_cast<uint16_t>(state.cpu.C));
}

void PtrC::set(State & state, uint8_t value)
{
	return state.set_memory_byte(0xFF00 + static_cast<uint16_t>(state.cpu.C), value);
}
