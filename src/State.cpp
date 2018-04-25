#include "State.hpp"
#include "Operations.hpp"
#include "OperationArguments.hpp"

#include <iostream>
#include <sstream>
#include <iomanip>

void State::log_op(uint16_t pc, const char * op)
{
	std::cout << "[0x" << std::uppercase << std::hex << std::setw(4) << std::setfill('0') << pc << "] "
			  << "0x" << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(this->get_memory_byte(pc)) << " "
			  << op << std::endl;
}

State::State() :
	cpu(),
	rom(),
	memory(),
	video(),
	is_error(false),
	interrupt_enabled(0),
	banks(),
	cycles(0)
{
	if (!this->rom.load("Pokemon Red.gb")) {
		this->is_error = true;
		return;
	}
	auto cartridge_info = get_cartridge_info(this->rom.cartridge());

	this->memory.resize(cartridge_info.memory_size);
	this->banks.resize(cartridge_info.bank_count);
}

[[noreturn]]
void State::panic(const std::string& message) const
{
	std::cout << "Panic raised" << std::endl;
	std::cout << message << std::endl;
	std::cout << *this << std::endl;
	exit(-1);
}

uint8_t State::consume_byte() {
	return 1;
}

uint8_t State::get_memory_byte(uint16_t addr) const
{
	if (addr == 0xFFFF) {
		// Interupt enable register
		return this->interrupt_enabled;
	}
	if (addr >= 0xFF80) {
		// Internal RAM
		this->panic("Internal RAM not implemented");
	}
	if (addr >= 0xFF4C) {
		// Empty but unusable for I/O
		std::ostringstream stream;
		stream << "Could not read from 0x" << std::uppercase << std::hex << addr << ", unused memory";
		this->panic(stream.str());
	}
	if (addr >= 0xFF00) {
		// I/O
		std::ostringstream stream;
		stream << "Could not read from 0x" << std::uppercase << std::hex << addr << ", I/O not implemented";
		this->panic(stream.str());
	}
	if (addr >= 0xFEA0) {
		// Empty but unusable for I/O
		std::ostringstream stream;
		stream << "Could not read from 0x" << std::uppercase << std::hex << addr << ", unused memory";
		this->panic(stream.str());
	}
	if (addr >= 0xFE00) {
		// Sprite Attrib Memory
		std::ostringstream stream;
		stream << "Could not read from 0x" << std::uppercase << std::hex << addr << ", Sprite Attrib memory not implemented";
		this->panic(stream.str());
	}
	if (addr >= 0xE000) {
		// Echo of 8kB internal ram
		return this->memory[addr - 0xE000];
	}
	if (addr >= 0xC000) {
		// Internal RAM
		return this->memory[addr - 0xC000];
	}
	if (addr >= 0xA000) {
		// Switchable RAM bank
		std::ostringstream stream;
		stream << "Could not read from 0x" << std::uppercase << std::hex << addr << ", switchable RAM bank not implemented";
		this->panic(stream.str());
	}
	if (addr >= 0x8000) {
		// Video Ram
		return this->video[addr - 0x8000];
	}
	return this->rom.get(addr);
}

void State::set_memory_byte(uint16_t addr, uint8_t value)
{
	if (addr == 0xFFFF) {
		// Interupt enable register
		this->interrupt_enabled = value;
	}
	else if (addr >= 0xFF80) {
		// Internal RAM
		this->panic("Internal RAM not implemented");
	}
	else if (addr >= 0xFF4C) {
		// Empty but unusable for I/O
		std::ostringstream stream;
		stream << "Could not write to 0x" << std::uppercase << std::hex << addr << ", unused memory";
		this->panic(stream.str());
	}
	else if (addr >= 0xFF00) {
		// I/O
		std::ostringstream stream;
		stream << "Could not write to 0x" << std::uppercase << std::hex << addr << ", I/O not implemented";
		this->panic(stream.str());
	}
	else if (addr >= 0xFEA0) {
		// Empty but unusable for I/O
		std::ostringstream stream;
		stream << "Could write to from 0x" << std::uppercase << std::hex << addr << ", unused memory";
		this->panic(stream.str());
	}
	else if (addr >= 0xFE00) {
		// Sprite Attrib Memory
		std::ostringstream stream;
		stream << "Could not write to 0x" << std::uppercase << std::hex << addr << ", Sprite Attrib memory not implemented";
		this->panic(stream.str());
	}
	else if (addr >= 0xE000) {
		// Echo of 8kB internal ram
		this->memory[addr - 0xE000] = value;
	}
	else if (addr >= 0xC000) {
		// Internal RAM
		this->memory[addr - 0xC000] = value;
	}
	else if (addr >= 0xA000) {
		// Switchable RAM bank
		std::ostringstream stream;
		stream << "Could not write to 0x" << std::uppercase << std::hex << addr << ", switchable RAM bank not implemented";
		this->panic(stream.str());
	}
	else if (addr >= 0x8000) {
		// Video Ram
		this->video[addr - 0x8000] = value;
	}
	else {
		this->panic("Attempting to write data at an invalid address");
	}
}

void State::process()
{
	auto pc = this->cpu.pc;
	this->cpu.pc++;
	auto op = this->get_memory_byte(pc);
	switch (op) {
	case 0x00:
		this->log_op(pc, "NOP");
		Nop::execute(*this);
		break;
	case 0x01:
		this->log_op(pc, "LD BC, d16");
		Load<BC, D16>::execute(*this);
		break;
	case 0x02:
		this->log_op(pc, "LD (BC), A");
		Load<PtrBC, A>::execute(*this);
		break;
	case 0x03:
		this->log_op(pc, "INC BC");
		Inc<BC>::execute(*this);
		break;
	case 0x04:
		this->log_op(pc, "INC B");
		Inc<B>::execute(*this);
		break;
	case 0x05:
		this->log_op(pc, "DEC B");
		Dec<B>::execute(*this);
		break;
	case 0x06:
		this->log_op(pc, "LD B, d8");
		Load<B, D8>::execute(*this);
		break;

	case 0x18:
		this->log_op(pc, "JR r8");
		JumpRelative<Always, D8>::execute(*this);
		break;

	case 0x28:
		this->log_op(pc, "JR Z, r8");
		JumpRelative<Z, D8>::execute(*this);
		break;

	case 0x48:
		this->log_op(pc, "LD C, B");
		Load<C, B>::execute(*this);
		break;
	case 0x49:
		this->log_op(pc, "LD C, C");
		Load<C, C>::execute(*this);
		break;

	case 0x54:
		this->log_op(pc, "LD D, H");
		Load<D, H>::execute(*this);
		break;
	case 0x5E:
		this->log_op(pc, "LD E, (HL)");
		Load<E, PtrHL>::execute(*this);
		break;

	case 0x62:
		this->log_op(pc, "LD H, D");
		Load<H, D>::execute(*this);
		break;
	case 0x64:
		this->log_op(pc, "LD H, H");
		Load<H, H>::execute(*this);
		break;
	case 0x65:
		this->log_op(pc, "LD H, L");
		Load<H, L>::execute(*this);
		break;
	case 0x67:
		this->log_op(pc, "LD H, A");
		Load<H, A>::execute(*this);
		break;

	case 0x83:
		this->log_op(pc, "ADD A,E");
		Add<A, E>::execute(*this);
		break;
	case 0x8e:
		this->log_op(pc, "ADC A,(HL)");
		AddCarry<A, PtrHL>::execute(*this);
		break;

	case 0xae:
		this->log_op(pc, "XOR (HL)");
		Xor<PtrHL>::execute(*this);
		break;

	case 0xaf:
		this->log_op(pc, "XOR A");
		Xor<A>::execute(*this);
		break;

	case 0xba:
		this->log_op(pc, "CP D");
		Compare<D>::execute(*this);
		break;

	case 0xc3:
		this->log_op(pc, "JMP A16");
		Jump<Always, D16>::execute(*this);
		break;

	case 0xf8:
		this->log_op(pc, "LD HL,SP+r8");
		Load<HL, SPR8>::execute(*this);
		break;
	case 0xfe:
		this->log_op(pc, "CP d8");
		Compare<D8>::execute(*this);
		break;
	default:
		std::stringstream stream;
		stream << "Could not process opcode 0x" << std::uppercase << std::hex << static_cast<uint32_t>(op);
		this->panic(stream.str());
	}
	std::cout << *this << std::endl;
}

std::ostream &operator<<(std::ostream &os, State const &state)
{
	return os
		<< "Cpu { "
		<< state.cpu
		<< " }";
}
