#include "State.hpp"
#include "Operations.hpp"
#include "OperationArguments.hpp"

#include <iostream>
#include <sstream>
#include <iomanip>

const uint8_t bootDMG[256] = {
	0x31, 0xFE, 0xFF, 0xAF, 0x21, 0xFF, 0x9F, 0x32, 0xCB, 0x7C, 0x20, 0xFB, 0x21, 0x26, 0xFF, 0x0E,
	0x11, 0x3E, 0x80, 0x32, 0xE2, 0x0C, 0x3E, 0xF3, 0xE2, 0x32, 0x3E, 0x77, 0x77, 0x3E, 0xFC, 0xE0,
	0x47, 0x11, 0x04, 0x01, 0x21, 0x10, 0x80, 0x1A, 0xCD, 0x95, 0x00, 0xCD, 0x96, 0x00, 0x13, 0x7B,
	0xFE, 0x34, 0x20, 0xF3, 0x11, 0xD8, 0x00, 0x06, 0x08, 0x1A, 0x13, 0x22, 0x23, 0x05, 0x20, 0xF9,
	0x3E, 0x19, 0xEA, 0x10, 0x99, 0x21, 0x2F, 0x99, 0x0E, 0x0C, 0x3D, 0x28, 0x08, 0x32, 0x0D, 0x20,
	0xF9, 0x2E, 0x0F, 0x18, 0xF3, 0x67, 0x3E, 0x64, 0x57, 0xE0, 0x42, 0x3E, 0x91, 0xE0, 0x40, 0x04,
	0x1E, 0x02, 0x0E, 0x0C, 0xF0, 0x44, 0xFE, 0x90, 0x20, 0xFA, 0x0D, 0x20, 0xF7, 0x1D, 0x20, 0xF2,
	0x0E, 0x13, 0x24, 0x7C, 0x1E, 0x83, 0xFE, 0x62, 0x28, 0x06, 0x1E, 0xC1, 0xFE, 0x64, 0x20, 0x06,
	0x7B, 0xE2, 0x0C, 0x3E, 0x87, 0xE2, 0xF0, 0x42, 0x90, 0xE0, 0x42, 0x15, 0x20, 0xD2, 0x05, 0x20,
	0x4F, 0x16, 0x20, 0x18, 0xCB, 0x4F, 0x06, 0x04, 0xC5, 0xCB, 0x11, 0x17, 0xC1, 0xCB, 0x11, 0x17,
	0x05, 0x20, 0xF5, 0x22, 0x23, 0x22, 0x23, 0xC9, 0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
	0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
	0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99, 0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC,
	0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E, 0x3C, 0x42, 0xB9, 0xA5, 0xB9, 0xA5, 0x42, 0x3C,
	0x21, 0x04, 0x01, 0x11, 0xA8, 0x00, 0x1A, 0x13, 0xBE, 0x00, 0x00, 0x23, 0x7D, 0xFE, 0x34, 0x20,
	0xF5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 0xFB, 0x86, 0x00, 0x00, 0x3E, 0x01, 0xE0, 0x50
};

void State::log_op(uint16_t pc, const char * op)
{
	return;
	std::cout << "[0x" << std::uppercase << std::hex << std::setw(4) << std::setfill('0') << pc << "] "
		<< "0x" << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << static_cast<uint32_t>(this->get_memory_byte(pc)) << " "
		<< op << std::endl;
}

State::State() :
	boot_rom_enabled(true),
	cpu(),
	rom(),
	io(),
	memory(),
	internal_memory(),
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

uint8_t State::get_memory_byte(uint16_t addr)
{
	if (addr == 0xFFFF) {
		// Interupt enable register
		return this->interrupt_enabled;
	}
	if (addr >= 0xFF80) {
		return this->internal_memory[addr - 0xFF80];
	}
	if (addr >= 0xFF4C) {
		// Empty but unusable for I/O
		std::ostringstream stream;
		stream << "Could not read from 0x" << std::uppercase << std::hex << addr << ", unused memory";
		this->panic(stream.str());
	}
	if (addr >= 0xFF00) {
		return this->io.get(addr - 0xFF00);
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
	if (addr >= 0x0100) {
		return this->rom.get(addr - 0x0100);
	}
	if (this->boot_rom_enabled) {
		return bootDMG[addr];
	}
	else {
		this->panic("Attempt to load from boot ROM after it's disabled");
	}
}

void State::set_memory_byte(uint16_t addr, uint8_t value)
{
	if (addr == 0xFFFF) {
		// Interupt enable register
		this->interrupt_enabled = value;
	}
	else if (addr >= 0xFF80) {
		this->internal_memory[addr - 0xFF80] = value;
	}
	else if (addr == 0xFF50) {
		std::cout << "Disabling boot ROM" << std::endl;
		this->boot_rom_enabled = false;
	}
	else if (addr >= 0xFF4C) {
		// Empty but unusable for I/O
		std::ostringstream stream;
		stream << "Could not write to 0x" << std::uppercase << std::hex << addr << ", unused memory";
		this->panic(stream.str());
	}
	else if (addr >= 0xFF00) {
		this->io.set(addr - 0xFF00, value);
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
	case 0x0C:
		this->log_op(pc, "INC C");
		Inc<C>::execute(*this);
		break;
	case 0x0D:
		this->log_op(pc, "DEC C");
		Dec<C>::execute(*this);
		break;
	case 0x0E:
		this->log_op(pc, "LD C, d8");
		Load<C, D8>::execute(*this);
		break;

	case 0x11:
		this->log_op(pc, "LD DE,d16");
		Load<DE, D16>::execute(*this);
		break;
	case 0x13:
		this->log_op(pc, "INC DE");
		Inc<DE>::execute(*this);
		break;
	case 0x15:
		this->log_op(pc, "DEC D");
		Dec<D>::execute(*this);
		break;
	case 0x16:
		this->log_op(pc, "LD D,d8");
		Load<D, D8>::execute(*this);
		break;
	case 0x17:
		this->log_op(pc, "RLA");
		RotateLeft<A>::execute(*this);
		break;
	case 0x18:
		this->log_op(pc, "JR r8");
		JumpRelative<Always, D8>::execute(*this);
		break;
	case 0x1A:
		this->log_op(pc, "LD A,(DE)");
		Load<A, PtrDE>::execute(*this);
		break;
	case 0x1D:
		this->log_op(pc, "DEC E");
		Dec<E>::execute(*this);
		break;
	case 0x1E:
		this->log_op(pc, "LD E,d8");
		Load<E, D8>::execute(*this);
		break;

	case 0x20:
		this->log_op(pc, "JR NZ,r8");
		JumpRelative<NZ, D8>::execute(*this);
		break;
	case 0x21:
		this->log_op(pc, "LD HL,d16");
		Load<HL, D16>::execute(*this);
		break;
	case 0x22:
		this->log_op(pc, "LD (HL+),A");
		Load<PtrHLIncr, A>::execute(*this);
		break;
	case 0x23:
		this->log_op(pc, "INC HL");
		Inc<HL>::execute(*this);
		break;
	case 0x24:
		this->log_op(pc, "INC H");
		Inc<H>::execute(*this);
		break;
	case 0x28:
		this->log_op(pc, "JR Z, r8");
		JumpRelative<Z, D8>::execute(*this);
		break;
	case 0x2E:
		this->log_op(pc, "LD L,d8");
		Load<L, D8>::execute(*this);
		break;

	case 0x31:
		this->log_op(pc, "LD SP,d16");
		Load<SP, D16>::execute(*this);
		break;
	case 0x32:
		this->log_op(pc, "LD (HL-),A");
		Load<PtrHLDecr, A>::execute(*this);
		break;
	case 0x3D:
		this->log_op(pc, "DEC A");
		Dec<A>::execute(*this);
		break;
	case 0x3E:
		this->log_op(pc, "LD A,d8");
		Load<A, D8>::execute(*this);
		break;

	case 0x48:
		this->log_op(pc, "LD C, B");
		Load<C, B>::execute(*this);
		break;
	case 0x49:
		this->log_op(pc, "LD C, C");
		Load<C, C>::execute(*this);
		break;
	case 0x4F:
		this->log_op(pc, "LD C, A");
		Load<C, A>::execute(*this);
		break;

	case 0x54:
		this->log_op(pc, "LD D, H");
		Load<D, H>::execute(*this);
		break;
	case 0x57:
		this->log_op(pc, "LD D,A");
		Load<D, A>::execute(*this);
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

	case 0x77:
		this->log_op(pc, "LD (HL),A");
		Load<PtrHL, A>::execute(*this);
		break;
	case 0x78:
		this->log_op(pc, "LD A,B");
		Load<A, B>::execute(*this);
		break;
	case 0x7B:
		this->log_op(pc, "LD A,E");
		Load<A, E>::execute(*this);
		break;
	case 0x7C:
		this->log_op(pc, "LD A,H");
		Load<A, H>::execute(*this);
		break;
	case 0x7D:
		this->log_op(pc, "LD A,L");
		Load<A, L>::execute(*this);
		break;

	case 0x83:
		this->log_op(pc, "ADD A,E");
		Add<A, E>::execute(*this);
		break;
	case 0x86:
		this->log_op(pc, "ADD A,(HL)");
		Add<A, PtrHL>::execute(*this);
		break;
	case 0x8E:
		this->log_op(pc, "ADC A,(HL)");
		AddCarry<A, PtrHL>::execute(*this);
		break;

	case 0x90:
		this->log_op(pc, "SUB B");
		Sub<B>::execute(*this);
		break;

	case 0xAE:
		this->log_op(pc, "XOR (HL)");
		Xor<PtrHL>::execute(*this);
		break;

	case 0xAF:
		this->log_op(pc, "XOR A");
		Xor<A>::execute(*this);
		break;

	case 0xBA:
		this->log_op(pc, "CP D");
		Compare<D>::execute(*this);
		break;
	case 0xBE:
		this->log_op(pc, "CP (HL)");
		Compare<PtrHL>::execute(*this);
		break;

	case 0xC1:
		this->log_op(pc, "POP BC");
		Pop<BC>::execute(*this);
		break;
	case 0xC3:
		this->log_op(pc, "JMP A16");
		Jump<Always, D16>::execute(*this);
		break;
	case 0xC5:
		this->log_op(pc, "PUSH BC");
		Push<BC>::execute(*this);
		break;
	case 0xC9:
		this->log_op(pc, "RET");
		Ret<Always>::execute(*this);
		break;
	case 0xCB:
		this->cycles += 4;
		this->process_cb();
		break;
	case 0xCD:
		this->log_op(pc, "CALL a16");
		Call<D16>::execute(*this);
		break;

	case 0xE0:
		this->log_op(pc, "LDH (a8), A");
		LoadIntoMemory<D8, A>::execute(*this);
		break;
	case 0xE2:
		this->log_op(pc, "LD (C), A");
		Load<PtrC, A>::execute(*this);
		break;
	case 0xEA:
		this->log_op(pc, "LD (a16),A");
		Load<PtrD16, A>::execute(*this);
		break;

	case 0xF0:
		this->log_op(pc, "LDH A,(a8)");
		Load<A, PtrD8>::execute(*this);
		break;
	case 0xF8:
		this->log_op(pc, "LD HL,SP+r8");
		Load<HL, SPR8>::execute(*this);
		break;
	case 0xFE:
		this->log_op(pc, "CP d8");
		Compare<D8>::execute(*this);
		break;

	default:
		std::stringstream stream;
		stream << "Could not process opcode 0x" << std::uppercase << std::hex << static_cast<uint32_t>(op);
		this->panic(stream.str());
	}
	// std::cout << *this << std::endl;
}

void State::process_cb() {
	auto pc = this->cpu.pc;
	this->cpu.pc++;
	auto op = this->get_memory_byte(pc);
	switch (op) {
	case 0x11:
		this->log_op(pc, "[CB] RL C");
		RotateLeft<C>::execute(*this);
		break;

	case 0x7c: 
		this->log_op(pc, "[CB] BIT 7, H");
		Bit<7, H>::execute(*this);
		break;

	default:
		std::stringstream stream;
		stream << "Could not process opcode [CB] 0x" << std::uppercase << std::hex << static_cast<uint32_t>(op);
		this->panic(stream.str());
	}
}

std::ostream &operator<<(std::ostream &os, State const &state)
{
	return os
		<< "Cpu { "
		<< state.cpu
		<< " }";
}
