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
}

bool State::load_rom(const char * rom)
{
	if (!this->rom.load(rom)) {
		this->is_error = true;
		return false;
	}
	auto cartridge_info = get_cartridge_info(this->rom.cartridge());

	this->memory.resize(cartridge_info.memory_size);
	this->banks.resize(cartridge_info.bank_count);
	return true;
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

struct Instruction {
	void(*op)(State&);
	const char* name;
	const uint8_t cycles;

	Instruction(const char* name, const uint8_t cycles) :
		op(nullptr),
		name(name),
		cycles(cycles)
	{
	}

	Instruction(void (*op)(State&), const char* name, const uint8_t cycles) :
		op(op),
		name(name),
		cycles(cycles)
	{
	}
};

const Instruction instructions[256] = {
	/* 0x00 */ { Nop::execute, "NOP", 4 },
	/* 0x01 */ { Load<BC, D16>::execute, "LD BC, d16", 12 },
	/* 0x02 */ { Load<PtrBC, A>::execute, "LD (BC), A", 8 },
	/* 0x03 */ { Inc<BC>::execute, "INC BC", 8 },
	/* 0x04 */ { Inc<B>::execute, "INC B", 4 },
	/* 0x05 */ { Dec<B>::execute, "DEC B", 4 },
	/* 0x06 */ { Load<B, D8>::execute, "LD B, d8", 8 },
	/* 0x07 */ { "RLCA", 4 },
	/* 0x08 */ { "LD (a16), SP", 20 },
	/* 0x09 */ { Add<HL, BC>::execute, "ADD HL, BC", 8 },
	/* 0x0A */ { Load<A, PtrBC>::execute, "LD A, (BC)", 8 },
	/* 0x0B */ { Dec<BC>::execute, "DEC BC", 8 },
	/* 0x0C */ { Inc<C>::execute, "INC C", 4 },
	/* 0x0D */ { Dec<C>::execute, "DEC C", 4 },
	/* 0x0E */ { Load<C, D8>::execute, "Load C, d8", 8 },
	/* 0x0F */ { "RRCA", 4 },

	/* 0x10 */{ "STOP", 4 },
	/* 0x11 */{ Load<DE, D16>::execute, "LD DE, d16", 12 },
	/* 0x12 */{ Load<PtrDE, A>::execute, "LD (DE), A", 8 },
	/* 0x13 */{ Inc<DE>::execute, "INC DE", 8 },
	/* 0x14 */{ Inc<D>::execute, "INC D", 4 },
	/* 0x15 */{ Dec<E>::execute, "DEC E", 4 },
	/* 0x16 */{ Load<D, D8>::execute, "LD D, d8", 8 },
	/* 0x17 */{ "RLA", 4 },
	/* 0x18 */{ JumpRelative<Always, D8>::execute, "JR D8", 8 },
	/* 0x19 */{ Add<HL, DE>::execute, "ADD HL, DE", 8 },
	/* 0x1A */{ Load<A, PtrDE>::execute, "LD A, (DE)", 8 },
	/* 0x1B */{ Dec<DE>::execute, "DEC DE", 8 },
	/* 0x1C */{ Inc<E>::execute, "INC E", 4 },
	/* 0x1D */{ Dec<E>::execute, "DEC E", 4 },
	/* 0x1E */{ Load<E, D8>::execute, "Load E, d8", 8 },
	/* 0x1F */{ "RRA", 4 },

	/* 0x20 */{ JumpRelative<NZ, D8>::execute, "JR NZ, R8", 8 },
	/* 0x21 */{ Load<HL, D16>::execute, "LD HL, d16", 12 },
	/* 0x22 */{ Load<PtrHLIncr, A>::execute, "LD (HL+), A", 8 },
	/* 0x23 */{ Inc<HL>::execute, "INC HL", 8 },
	/* 0x24 */{ Inc<H>::execute, "INC H", 4 },
	/* 0x25 */{ Dec<H>::execute, "DEC H", 4 },
	/* 0x26 */{ Load<H, D8>::execute, "LD H, d8", 8 },
	/* 0x27 */{ "DAA", 4 },
	/* 0x28 */{ JumpRelative<Z, D8>::execute, "JR Z, D8", 8 },
	/* 0x29 */{ Add<HL, HL>::execute, "ADD HL, HL", 8 },
	/* 0x2A */{ Load<A, PtrHLIncr>::execute, "LD A, (HL+)", 8 },
	/* 0x2B */{ Dec<HL>::execute, "DEC HL", 8 },
	/* 0x2C */{ Inc<L>::execute, "INC L", 4 },
	/* 0x2D */{ Dec<L>::execute, "DEC L", 4 },
	/* 0x2E */{ Load<L, D8>::execute, "Load L, d8", 8 },
	/* 0x2F */{ "CPL", 4 },

	/* 0x30 */{ JumpRelative<NC, D8>::execute, "JR NC, R8", 8 },
	/* 0x31 */{ Load<SP, D16>::execute, "LD SP, d16", 12 },
	/* 0x32 */{ Load<PtrHLDecr, A>::execute, "LD (HL-), A", 8 },
	/* 0x33 */{ Inc<SP>::execute, "INC SP", 8 },
	/* 0x34 */{ Inc<PtrHL>::execute, "INC (HL)", 12 },
	/* 0x35 */{ Dec<PtrHL>::execute, "DEC (HL)", 12 },
	/* 0x36 */{ Load<PtrHL, D8>::execute, "LD H, d8", 12 },
	/* 0x37 */{ "SCF", 4 },
	/* 0x38 */{ JumpRelative<C, D8>::execute, "JR C, D8", 8 },
	/* 0x39 */{ Add<HL, SP>::execute, "ADD HL, SP", 8 },
	/* 0x3A */{ Load<A, PtrHLDecr>::execute, "LD A, (HL-)", 8 },
	/* 0x3B */{ Dec<SP>::execute, "DEC SP", 8 },
	/* 0x3C */{ Inc<A>::execute, "INC A", 4 },
	/* 0x3D */{ Dec<A>::execute, "DEC A", 4 },
	/* 0x3E */{ Load<A, D8>::execute, "Load A, d8", 8 },
	/* 0x3F */{ "CCF", 4 },

	/* 0x40 */{ Load<B, B>::execute, "LD B, B", 4 },
	/* 0x41 */{ Load<B, C>::execute, "LD B, C", 4 },
	/* 0x42 */{ Load<B, D>::execute, "LD B, D", 4 },
	/* 0x43 */{ Load<B, E>::execute, "LD B, E", 4 },
	/* 0x44 */{ Load<B, H>::execute, "LD B, H", 4 },
	/* 0x45 */{ Load<B, L>::execute, "LD B, L", 4 },
	/* 0x46 */{ Load<B, PtrHL>::execute, "LD B, (HL)", 8 },
	/* 0x47 */{ Load<B, A>::execute, "LD B, A", 4 },
	/* 0x48 */{ Load<C, B>::execute, "LD C, B", 4 },
	/* 0x49 */{ Load<C, C>::execute, "LD C, C", 4 },
	/* 0x4A */{ Load<C, D>::execute, "LD C, D", 4 },
	/* 0x4B */{ Load<C, E>::execute, "LD C, E", 4 },
	/* 0x4C */{ Load<C, H>::execute, "LD C, H", 4 },
	/* 0x4D */{ Load<C, L>::execute, "LD C, L", 4 },
	/* 0x4E */{ Load<C, PtrHL>::execute, "LD C, (HL)", 8 },
	/* 0x4F */{ Load<C, A>::execute, "LD C, A", 4 },

	/* 0x50 */{ Load<D, B>::execute, "LD D, B", 4 },
	/* 0x51 */{ Load<D, C>::execute, "LD D, C", 4 },
	/* 0x52 */{ Load<D, D>::execute, "LD D, D", 4 },
	/* 0x53 */{ Load<D, E>::execute, "LD D, E", 4 },
	/* 0x54 */{ Load<D, H>::execute, "LD D, H", 4 },
	/* 0x55 */{ Load<D, L>::execute, "LD D, L", 4 },
	/* 0x56 */{ Load<D, PtrHL>::execute, "LD D, (HL)", 8 },
	/* 0x57 */{ Load<D, A>::execute, "LD D, A", 4 },
	/* 0x58 */{ Load<E, B>::execute, "LD E, B", 4 },
	/* 0x59 */{ Load<E, C>::execute, "LD E, C", 4 },
	/* 0x5A */{ Load<E, D>::execute, "LD E, D", 4 },
	/* 0x5B */{ Load<E, E>::execute, "LD E, E", 4 },
	/* 0x5C */{ Load<E, H>::execute, "LD E, H", 4 },
	/* 0x5D */{ Load<E, L>::execute, "LD E, L", 4 },
	/* 0x5E */{ Load<E, PtrHL>::execute, "LD E, (HL)", 8 },
	/* 0x5F */{ Load<E, A>::execute, "LD E, A", 4 },

	/* 0x60 */{ Load<H, B>::execute, "LD H, B", 4 },
	/* 0x61 */{ Load<H, C>::execute, "LD H, C", 4 },
	/* 0x62 */{ Load<H, D>::execute, "LD H, D", 4 },
	/* 0x63 */{ Load<H, E>::execute, "LD H, E", 4 },
	/* 0x64 */{ Load<H, H>::execute, "LD H, H", 4 },
	/* 0x65 */{ Load<H, L>::execute, "LD H, L", 4 },
	/* 0x66 */{ Load<H, PtrHL>::execute, "LD H, (HL)", 8 },
	/* 0x67 */{ Load<H, A>::execute, "LD H, A", 4 },
	/* 0x68 */{ Load<L, B>::execute, "LD L, B", 4 },
	/* 0x69 */{ Load<L, C>::execute, "LD L, C", 4 },
	/* 0x6A */{ Load<L, D>::execute, "LD L, D", 4 },
	/* 0x6B */{ Load<L, E>::execute, "LD L, E", 4 },
	/* 0x6C */{ Load<L, H>::execute, "LD L, H", 4 },
	/* 0x6D */{ Load<L, L>::execute, "LD L, L", 4 },
	/* 0x6E */{ Load<L, PtrHL>::execute, "LD L, (HL)", 8 },
	/* 0x6F */{ Load<L, A>::execute, "LD L, A", 4 },

	/* 0x70 */{ Load<PtrHL, B>::execute, "LD (HL), B", 8 },
	/* 0x71 */{ Load<PtrHL, C>::execute, "LD (HL), C", 8 },
	/* 0x72 */{ Load<PtrHL, D>::execute, "LD (HL), D", 8 },
	/* 0x73 */{ Load<PtrHL, E>::execute, "LD (HL), E", 8 },
	/* 0x74 */{ Load<PtrHL, H>::execute, "LD (HL), H", 8 },
	/* 0x75 */{ Load<PtrHL, L>::execute, "LD (HL), L", 8 },
	/* 0x76 */{ "HALT", 4 },
	/* 0x77 */{ Load<PtrHL, A>::execute, "LD (HL), A", 8 },
	/* 0x78 */{ Load<A, B>::execute, "LD A, B", 4 },
	/* 0x79 */{ Load<A, C>::execute, "LD A, C", 4 },
	/* 0x7A */{ Load<A, D>::execute, "LD A, D", 4 },
	/* 0x7B */{ Load<A, E>::execute, "LD A, E", 4 },
	/* 0x7C */{ Load<A, H>::execute, "LD A, H", 4 },
	/* 0x7D */{ Load<A, L>::execute, "LD A, L", 4 },
	/* 0x7E */{ Load<A, PtrHL>::execute, "LD A, (HL)", 8 },
	/* 0x7F */{ Load<A, A>::execute, "LD A, A", 4 },

	/* 0x80 */{ Add<A, B>::execute, "ADD A, B", 4 },
	/* 0x81 */{	Add<A, C>::execute, "ADD A, C", 4 },
	/* 0x82 */{	Add<A, D>::execute, "ADD A, D", 4 },
	/* 0x83 */{	Add<A, E>::execute, "ADD A, E", 4 },
	/* 0x84 */{	Add<A, H>::execute, "ADD A, H", 4 },
	/* 0x85 */{	Add<A, L>::execute, "ADD A, L", 4 },
	/* 0x86 */{	Add<A, PtrHL>::execute, "ADD A, (HL)", 8 },
	/* 0x87 */{	Add<A, A>::execute, "ADD A, A", 4 },
	/* 0x88 */{	AddCarry<A, B>::execute, "ADC A, B", 4 },
	/* 0x89 */{	AddCarry<A, C>::execute, "ADC A, C", 4 },
	/* 0x8A */{	AddCarry<A, D>::execute, "ADC A, D", 4 },
	/* 0x8B */{	AddCarry<A, E>::execute, "ADC A, E", 4 },
	/* 0x8C */{	AddCarry<A, H>::execute, "ADC A, H", 4 },
	/* 0x8D */{	AddCarry<A, L>::execute, "ADC A, L", 4 },
	/* 0x8E */{	AddCarry<A, PtrHL>::execute, "ADC A, (HL)", 8 },
	/* 0x8F */{	AddCarry<A, A>::execute, "ADC A, A", 4 },

	/* 0x90 */{ Sub<B>::execute, "SUB B", 4 },
	/* 0x91 */{ Sub<C>::execute, "SUB C", 4 },
	/* 0x92 */{ Sub<D>::execute, "SUB D", 4 },
	/* 0x93 */{ Sub<E>::execute, "SUB E", 4 },
	/* 0x94 */{ Sub<H>::execute, "SUB H", 4 },
	/* 0x95 */{ Sub<L>::execute, "SUB L", 4 },
	/* 0x96 */{ Sub<PtrHL>::execute, "SUB (HL)", 8 },
	/* 0x97 */{ Sub<A>::execute, "SUB A", 4 },
	/* 0x98 */{ SubCarry<B>::execute, "SBC B", 4 },
	/* 0x99 */{ SubCarry<C>::execute, "SBC C", 4 },
	/* 0x9A */{ SubCarry<D>::execute, "SBC D", 4 },
	/* 0x9B */{ SubCarry<E>::execute, "SBC E", 4 },
	/* 0x9C */{ SubCarry<H>::execute, "SBC H", 4 },
	/* 0x9D */{ SubCarry<L>::execute, "SBC L", 4 },
	/* 0x9E */{ SubCarry<PtrHL>::execute, "SBC (HL)", 8 },
	/* 0x9F */{ SubCarry<A>::execute, "SBC A", 4 },

	/* 0xA0 */{ And<B>::execute, "AND B", 4 },
	/* 0xA1 */{ And<C>::execute, "AND C", 4 },
	/* 0xA2 */{ And<D>::execute, "AND D", 4 },
	/* 0xA3 */{ And<E>::execute, "AND E", 4 },
	/* 0xA4 */{ And<H>::execute, "AND H", 4 },
	/* 0xA5 */{ And<L>::execute, "AND L", 4 },
	/* 0xA6 */{ And<PtrHL>::execute, "AND (HL)", 8 },
	/* 0xA7 */{ And<A>::execute, "AND A", 4 },
	/* 0xA8 */{ Xor<B>::execute, "XOR B", 4 },
	/* 0xA9 */{ Xor<C>::execute, "XOR C", 4 },
	/* 0xAA */{ Xor<D>::execute, "XOR D", 4 },
	/* 0xAB */{ Xor<E>::execute, "XOR E", 4 },
	/* 0xAC */{ Xor<H>::execute, "XOR H", 4 },
	/* 0xAD */{ Xor<L>::execute, "XOR L", 4 },
	/* 0xAE */{ Xor<PtrHL>::execute, "XOR (HL)", 8 },
	/* 0xAF */{ Xor<A>::execute, "XOR A", 4 },

	/* 0xB0 */{ Or<B>::execute, "OR B", 4 },
	/* 0xB1 */{ Or<C>::execute, "OR C", 4 },
	/* 0xB2 */{ Or<D>::execute, "OR D", 4 },
	/* 0xB3 */{ Or<E>::execute, "OR E", 4 },
	/* 0xB4 */{ Or<H>::execute, "OR H", 4 },
	/* 0xB5 */{ Or<L>::execute, "OR L", 4 },
	/* 0xB6 */{ Or<PtrHL>::execute, "OR (HL)", 8 },
	/* 0xB7 */{ Or<A>::execute, "OR A", 4 },
	/* 0xB8 */{ Compare<B>::execute, "CP B", 4 },
	/* 0xB9 */{ Compare<C>::execute, "CP C", 4 },
	/* 0xBA */{ Compare<D>::execute, "CP D", 4 },
	/* 0xBB */{ Compare<E>::execute, "CP E", 4 },
	/* 0xBC */{ Compare<H>::execute, "CP H", 4 },
	/* 0xBD */{ Compare<L>::execute, "CP L", 4 },
	/* 0xBE */{ Compare<PtrHL>::execute, "CP (HL)", 8 },
	/* 0xBF */{ Compare<A>::execute, "CP A", 4 },

	/* 0xC0 */{ Ret<NZ>::execute, "RET NZ", 8 },
	/* 0xC1 */{ Pop<BC>::execute, "POP BC", 12 },
	/* 0xC2 */{ Jump<NZ, D16>::execute, "JP NZ, a16", 12 },
	/* 0xC3 */{ Jump<Always, D16>::execute, "JP A16", 12 },
	/* 0xC4 */{ Call<NZ, D16>::execute, "CALL NZ, A16", 12 },
	/* 0xC5 */{ Push<BC>::execute, "PUSH BC", 16 },
	/* 0xC6 */{ Add<A, D8>::execute, "ADD A, d8", 8 },
	/* 0xC7 */{ "RST 00H", 16 },
	/* 0xC8 */{ Ret<Z>::execute, "RET Z", 8 },
	/* 0xC9 */{ Ret<Always>::execute, "RET", 4 },
	/* 0xCA */{ Jump<Z, D16>::execute, "JP Z, a16", 12 },
	/* 0xCB */{ PrefixCB::execute, "PREFIX CB", 4 },
	/* 0xCC */{ Call<Z, D16>::execute, "CALL Z, a16", 12 },
	/* 0xCD */{ Call<Always, D16>::execute, "CALL a16", 12 },
	/* 0xCE */{ AddCarry<A, D8>::execute, "ADC A, d8", 8 },
	/* 0xCF */{ "RST 08H", 16 },

	/* 0xD0 */{ Ret<NC>::execute, "RET NC", 8 },
	/* 0xD1 */{ Pop<DE>::execute, "POP DE", 12 },
	/* 0xD2 */{ Jump<NC, D16>::execute, "JP NC, a16", 12 },
	/* 0xD3 */{ "Undefined", 0 },
	/* 0xD4 */{ Call<NC, D16>::execute, "CALL NC, A16", 12 },
	/* 0xD5 */{ Push<DE>::execute, "PUSH DE", 16 },
	/* 0xD6 */{ Sub<D8>::execute, "SUB A, d8", 8 },
	/* 0xD7 */{ "RST 10H", 16 },
	/* 0xD8 */{ Ret<C>::execute, "RET C", 8 },
	/* 0xD9 */{ "RETI (Return interrupt)", 16 },
	/* 0xDA */{ Jump<C, D16>::execute, "JP C, a16", 12 },
	/* 0xDB */{ "Undefined", 0 },
	/* 0xDC */{ Call<C, D16>::execute, "CALL C, a16", 12 },
	/* 0xDD */{ "Undefined", 0 },
	/* 0xDE */{ SubCarry<D8>::execute, "SBC A, d8", 8 },
	/* 0xDF */{ "RST 18H", 16 },

	/* 0xE0 */{ "LDH (a8), A", 12 },
	/* 0xE1 */{ Pop<HL>::execute, "POP HL", 12 },
	/* 0xE2 */{ Load<PtrC, A>::execute, "LD (C), A", 8 },
	/* 0xE3 */{ "Undefined", 0 },
	/* 0xE4 */{ "Undefined", 0 },
	/* 0xE5 */{ Push<HL>::execute, "PUSH HL", 16 },
	/* 0xE6 */{ And<D8>::execute, "AND A, d8", 8 },
	/* 0xE7 */{ "RST 20H", 16 },
	/* 0xE8 */{ Add<SP, D8>::execute, "Add SP, r8", 16 },
	/* 0xE9 */{ Jump<Always, PtrHL>::execute, "JP (HL)", 4 },
	/* 0xEA */{ Load<PtrD16, A>::execute, "LD (a16), A", 16 },
	/* 0xEB */{ "Undefined", 0 },
	/* 0xEC */{ "Undefined", 0 },
	/* 0xED */{ "Undefined", 0 },
	/* 0xEE */{ Xor<D8>::execute, "XOR A, d8", 8 },
	/* 0xEF */{ "RST 28H", 16 },

	/* 0xF0 */{ "LDH A, (a8)", 12 },
	/* 0xF1 */{ Pop<AF>::execute, "POP AF", 12 },
	/* 0xF2 */{ Load<A, PtrC>::execute, "LD A, (C)", 8 },
	/* 0xF3 */{ "DI", 4 },
	/* 0xF4 */{ "Undefined", 0 },
	/* 0xF5 */{ Push<AF>::execute, "PUSH AF", 16 },
	/* 0xF6 */{ Or<D8>::execute, "OR A, d8", 8 },
	/* 0xF7 */{ "RST 30H", 16 },
	/* 0xF8 */{ Load<HL, SPR8>::execute, "LD HL, SP + R8", 12 },
	/* 0xF9 */{ Load<SP, HL>::execute, "LD SP, HL", 4 },
	/* 0xFA */{ Load<A, PtrD16>::execute, "LD A, (a16)", 16 },
	/* 0xFB */{ "EI", 4 },
	/* 0xFC */{ "Undefined", 0 },
	/* 0xFD */{ "Undefined", 0 },
	/* 0xFE */{ Compare<D8>::execute, "CP A, d8", 8 },
	/* 0xFF */{ "RST 38H", 16 },
};
const Instruction extended_instructions[256] = {
	/* 0x00 */{ RotateLeftCarry<B>::execute, "RLC B", 8 },
	/* 0x01 */{ RotateLeftCarry<C>::execute, "RLC C", 8 },
	/* 0x02 */{ RotateLeftCarry<D>::execute, "RLC D", 8 },
	/* 0x03 */{ RotateLeftCarry<E>::execute, "RLC E", 8 },
	/* 0x04 */{ RotateLeftCarry<H>::execute, "RLC H", 8 },
	/* 0x05 */{ RotateLeftCarry<L>::execute, "RLC L", 8 },
	/* 0x06 */{ RotateLeftCarry<PtrHL>::execute, "RLC (HL)", 16 },
	/* 0x07 */{ RotateLeftCarry<A>::execute, "RLC A", 8 },
	/* 0x08 */{ RotateRightCarry<B>::execute, "RRC B", 8 },
	/* 0x09 */{ RotateRightCarry<C>::execute, "RRC C", 8 },
	/* 0x0A */{ RotateRightCarry<D>::execute, "RRC D", 8 },
	/* 0x0B */{ RotateRightCarry<E>::execute, "RRC E", 8 },
	/* 0x0C */{ RotateRightCarry<H>::execute, "RRC H", 8 },
	/* 0x0D */{ RotateRightCarry<L>::execute, "RRC L", 8 },
	/* 0x0E */{ RotateRightCarry<PtrHL>::execute, "RRC (HL)", 12 },
	/* 0x0F */{ RotateRightCarry<A>::execute, "RRC A", 8 },

	/* 0x10 */{ RotateLeft<B>::execute, "RL B", 8 },
	/* 0x11 */{ RotateLeft<C>::execute, "RL C", 8 },
	/* 0x12 */{ RotateLeft<D>::execute, "RL D", 8 },
	/* 0x13 */{ RotateLeft<E>::execute, "RL E", 8 },
	/* 0x14 */{ RotateLeft<H>::execute, "RL H", 8 },
	/* 0x15 */{ RotateLeft<L>::execute, "RL L", 8 },
	/* 0x16 */{ RotateLeft<PtrHL>::execute, "RL (HL)", 16 },
	/* 0x17 */{ RotateLeft<A>::execute, "RL A", 8 },
	/* 0x18 */{ RotateRight<B>::execute, "RR B", 8 },
	/* 0x19 */{ RotateRight<C>::execute, "RR C", 8 },
	/* 0x1A */{ RotateRight<D>::execute, "RR D", 8 },
	/* 0x1B */{ RotateRight<E>::execute, "RR E", 8 },
	/* 0x1C */{ RotateRight<H>::execute, "RR H", 8 },
	/* 0x1D */{ RotateRight<L>::execute, "RR L", 8 },
	/* 0x1E */{ RotateRight<PtrHL>::execute, "RR (HL)", 12 },
	/* 0x1F */{ RotateRight<A>::execute, "RR A", 8 },

	/* 0x20 */{ ShiftLeft<B>::execute, "SLA B", 8 },
	/* 0x21 */{ ShiftLeft<C>::execute, "SLA C", 8 },
	/* 0x22 */{ ShiftLeft<D>::execute, "SLA D", 8 },
	/* 0x23 */{ ShiftLeft<E>::execute, "SLA E", 8 },
	/* 0x24 */{ ShiftLeft<H>::execute, "SLA H", 8 },
	/* 0x25 */{ ShiftLeft<L>::execute, "SLA L", 8 },
	/* 0x26 */{ ShiftLeft<PtrHL>::execute, "SLA (HL)", 16 },
	/* 0x27 */{ ShiftLeft<A>::execute, "SLA A", 8 },
	/* 0x28 */{ ShiftRight<B>::execute, "SRA B", 8 },
	/* 0x29 */{ ShiftRight<C>::execute, "SRA C", 8 },
	/* 0x2A */{ ShiftRight<D>::execute, "SRA D", 8 },
	/* 0x2B */{ ShiftRight<E>::execute, "SRA E", 8 },
	/* 0x2C */{ ShiftRight<H>::execute, "SRA H", 8 },
	/* 0x2D */{ ShiftRight<L>::execute, "SRA L", 8 },
	/* 0x2E */{ ShiftRight<PtrHL>::execute, "SRA (HL)", 12 },
	/* 0x2F */{ ShiftRight<A>::execute, "SRA A", 8 },

	/* 0x30 */{ Swap<B>::execute, "Swap B", 8 },
	/* 0x31 */{ Swap<C>::execute, "Swap C", 8 },
	/* 0x32 */{ Swap<D>::execute, "Swap D", 8 },
	/* 0x33 */{ Swap<E>::execute, "Swap E", 8 },
	/* 0x34 */{ Swap<H>::execute, "Swap H", 8 },
	/* 0x35 */{ Swap<L>::execute, "Swap L", 8 },
	/* 0x36 */{ Swap<PtrHL>::execute, "Swap (HL)", 16 },
	/* 0x37 */{ Swap<A>::execute, "Swap A", 8 },
	/* 0x38 */{ ShiftRightClear<B>::execute, "SRL B", 8 },
	/* 0x39 */{ ShiftRightClear<C>::execute, "SRL C", 8 },
	/* 0x3A */{ ShiftRightClear<D>::execute, "SRL D", 8 },
	/* 0x3B */{ ShiftRightClear<E>::execute, "SRL E", 8 },
	/* 0x3C */{ ShiftRightClear<H>::execute, "SRL H", 8 },
	/* 0x3D */{ ShiftRightClear<L>::execute, "SRL L", 8 },
	/* 0x3E */{ ShiftRightClear<PtrHL>::execute, "SRL (HL)", 12 },
	/* 0x3F */{ ShiftRightClear<A>::execute, "SRL A", 8 },

	/* 0x40 */{ Bit<0, B>::execute, "BIT 0 B", 8 },
	/* 0x41 */{ Bit<0, C>::execute, "BIT 0 C", 8 },
	/* 0x42 */{ Bit<0, D>::execute, "BIT 0 D", 8 },
	/* 0x43 */{ Bit<0, E>::execute, "BIT 0 E", 8 },
	/* 0x44 */{ Bit<0, H>::execute, "BIT 0 H", 8 },
	/* 0x45 */{ Bit<0, L>::execute, "BIT 0 L", 8 },
	/* 0x46 */{ Bit<0, PtrHL>::execute, "BIT 0 (HL)", 16 },
	/* 0x47 */{ Bit<0, A>::execute, "BIT 0 A", 8 },
	/* 0x48 */{ Bit<1, B>::execute, "BIT 1 B", 8 },
	/* 0x49 */{ Bit<1, C>::execute, "BIT 1 C", 8 },
	/* 0x4A */{ Bit<1, D>::execute, "BIT 1 D", 8 },
	/* 0x4B */{ Bit<1, E>::execute, "BIT 1 E", 8 },
	/* 0x4C */{ Bit<1, H>::execute, "BIT 1 H", 8 },
	/* 0x4D */{ Bit<1, L>::execute, "BIT 1 L", 8 },
	/* 0x4E */{ Bit<1, PtrHL>::execute, "BIT 1 (HL)", 12 },
	/* 0x4F */{ Bit<1, A>::execute, "BIT 1 A", 8 },

	/* 0x50 */{ Bit<2, B>::execute, "BIT 2 B", 8 },
	/* 0x51 */{ Bit<2, C>::execute, "BIT 2 C", 8 },
	/* 0x52 */{ Bit<2, D>::execute, "BIT 2 D", 8 },
	/* 0x53 */{ Bit<2, E>::execute, "BIT 2 E", 8 },
	/* 0x54 */{ Bit<2, H>::execute, "BIT 2 H", 8 },
	/* 0x55 */{ Bit<2, L>::execute, "BIT 2 L", 8 },
	/* 0x56 */{ Bit<2, PtrHL>::execute, "BIT 2 (HL)", 16 },
	/* 0x57 */{ Bit<2, A>::execute, "BIT 2 A", 8 },
	/* 0x58 */{ Bit<3, B>::execute, "BIT 3 B", 8 },
	/* 0x59 */{ Bit<3, C>::execute, "BIT 3 C", 8 },
	/* 0x5A */{ Bit<3, D>::execute, "BIT 3 D", 8 },
	/* 0x5B */{ Bit<3, E>::execute, "BIT 3 E", 8 },
	/* 0x5C */{ Bit<3, H>::execute, "BIT 3 H", 8 },
	/* 0x5D */{ Bit<3, L>::execute, "BIT 3 L", 8 },
	/* 0x5E */{ Bit<3, PtrHL>::execute, "BIT 3 (HL)", 12 },
	/* 0x5F */{ Bit<3, A>::execute, "BIT 3 A", 8 },

	/* 0x60 */{ Bit<4, B>::execute, "BIT 4 B", 8 },
	/* 0x61 */{ Bit<4, C>::execute, "BIT 4 C", 8 },
	/* 0x62 */{ Bit<4, D>::execute, "BIT 4 D", 8 },
	/* 0x63 */{ Bit<4, E>::execute, "BIT 4 E", 8 },
	/* 0x64 */{ Bit<4, H>::execute, "BIT 4 H", 8 },
	/* 0x65 */{ Bit<4, L>::execute, "BIT 4 L", 8 },
	/* 0x66 */{ Bit<4, PtrHL>::execute, "BIT 4 (HL)", 16 },
	/* 0x67 */{ Bit<4, A>::execute, "BIT 4 A", 8 },
	/* 0x68 */{ Bit<5, B>::execute, "BIT 5 B", 8 },
	/* 0x69 */{ Bit<5, C>::execute, "BIT 5 C", 8 },
	/* 0x6A */{ Bit<5, D>::execute, "BIT 5 D", 8 },
	/* 0x6B */{ Bit<5, E>::execute, "BIT 5 E", 8 },
	/* 0x6C */{ Bit<5, H>::execute, "BIT 5 H", 8 },
	/* 0x6D */{ Bit<5, L>::execute, "BIT 5 L", 8 },
	/* 0x6E */{ Bit<5, PtrHL>::execute, "BIT 5 (HL)", 12 },
	/* 0x6F */{ Bit<5, A>::execute, "BIT 5 A", 8 },

	/* 0x70 */{ Bit<6, B>::execute, "BIT 6 B", 8 },
	/* 0x71 */{ Bit<6, C>::execute, "BIT 6 C", 8 },
	/* 0x72 */{ Bit<6, D>::execute, "BIT 6 D", 8 },
	/* 0x73 */{ Bit<6, E>::execute, "BIT 6 E", 8 },
	/* 0x74 */{ Bit<6, H>::execute, "BIT 6 H", 8 },
	/* 0x75 */{ Bit<6, L>::execute, "BIT 6 L", 8 },
	/* 0x76 */{ Bit<6, PtrHL>::execute, "BIT 6 (HL)", 16 },
	/* 0x77 */{ Bit<6, A>::execute, "BIT 6 A", 8 },
	/* 0x78 */{ Bit<7, B>::execute, "BIT 7 B", 8 },
	/* 0x79 */{ Bit<7, C>::execute, "BIT 7 C", 8 },
	/* 0x7A */{ Bit<7, D>::execute, "BIT 7 D", 8 },
	/* 0x7B */{ Bit<7, E>::execute, "BIT 7 E", 8 },
	/* 0x7C */{ Bit<7, H>::execute, "BIT 7 H", 8 },
	/* 0x7D */{ Bit<7, L>::execute, "BIT 7 L", 8 },
	/* 0x7E */{ Bit<7, PtrHL>::execute, "BIT 7 (HL)", 12 },
	/* 0x7F */{ Bit<7, A>::execute, "BIT 7 A", 8 },

	/* 0x80 */{ ClearBit<0, B>::execute, "RES 0 B", 8 },
	/* 0x81 */{ ClearBit<0, C>::execute, "RES 0 C", 8 },
	/* 0x82 */{ ClearBit<0, D>::execute, "RES 0 D", 8 },
	/* 0x83 */{ ClearBit<0, E>::execute, "RES 0 E", 8 },
	/* 0x84 */{ ClearBit<0, H>::execute, "RES 0 H", 8 },
	/* 0x85 */{ ClearBit<0, L>::execute, "RES 0 L", 8 },
	/* 0x86 */{ ClearBit<0, PtrHL>::execute, "RES 0 (HL)", 16 },
	/* 0x87 */{ ClearBit<0, A>::execute, "RES 0 A", 8 },
	/* 0x88 */{ ClearBit<1, B>::execute, "RES 1 B", 8 },
	/* 0x89 */{ ClearBit<1, C>::execute, "RES 1 C", 8 },
	/* 0x8A */{ ClearBit<1, D>::execute, "RES 1 D", 8 },
	/* 0x8B */{ ClearBit<1, E>::execute, "RES 1 E", 8 },
	/* 0x8C */{ ClearBit<1, H>::execute, "RES 1 H", 8 },
	/* 0x8D */{ ClearBit<1, L>::execute, "RES 1 L", 8 },
	/* 0x8E */{ ClearBit<1, PtrHL>::execute, "RES 1 (HL)", 12 },
	/* 0x8F */{ ClearBit<1, A>::execute, "RES 1 A", 8 },

	/* 0x90 */{ ClearBit<2, B>::execute, "RES 2 B", 8 },
	/* 0x91 */{ ClearBit<2, C>::execute, "RES 2 C", 8 },
	/* 0x92 */{ ClearBit<2, D>::execute, "RES 2 D", 8 },
	/* 0x93 */{ ClearBit<2, E>::execute, "RES 2 E", 8 },
	/* 0x94 */{ ClearBit<2, H>::execute, "RES 2 H", 8 },
	/* 0x95 */{ ClearBit<2, L>::execute, "RES 2 L", 8 },
	/* 0x96 */{ ClearBit<2, PtrHL>::execute, "RES 2 (HL)", 16 },
	/* 0x97 */{ ClearBit<2, A>::execute, "RES 2 A", 8 },
	/* 0x98 */{ ClearBit<3, B>::execute, "RES 3 B", 8 },
	/* 0x99 */{ ClearBit<3, C>::execute, "RES 3 C", 8 },
	/* 0x9A */{ ClearBit<3, D>::execute, "RES 3 D", 8 },
	/* 0x9B */{ ClearBit<3, E>::execute, "RES 3 E", 8 },
	/* 0x9C */{ ClearBit<3, H>::execute, "RES 3 H", 8 },
	/* 0x9D */{ ClearBit<3, L>::execute, "RES 3 L", 8 },
	/* 0x9E */{ ClearBit<3, PtrHL>::execute, "RES 3 (HL)", 12 },
	/* 0x9F */{ ClearBit<3, A>::execute, "RES 3 A", 8 },

	/* 0xA0 */{ ClearBit<4, B>::execute, "RES 4 B", 8 },
	/* 0xA1 */{ ClearBit<4, C>::execute, "RES 4 C", 8 },
	/* 0xA2 */{ ClearBit<4, D>::execute, "RES 4 D", 8 },
	/* 0xA3 */{ ClearBit<4, E>::execute, "RES 4 E", 8 },
	/* 0xA4 */{ ClearBit<4, H>::execute, "RES 4 H", 8 },
	/* 0xA5 */{ ClearBit<4, L>::execute, "RES 4 L", 8 },
	/* 0xA6 */{ ClearBit<4, PtrHL>::execute, "RES 4 (HL)", 16 },
	/* 0xA7 */{ ClearBit<4, A>::execute, "RES 4 A", 8 },
	/* 0xA8 */{ ClearBit<5, B>::execute, "RES 5 B", 8 },
	/* 0xA9 */{ ClearBit<5, C>::execute, "RES 5 C", 8 },
	/* 0xAA */{ ClearBit<5, D>::execute, "RES 5 D", 8 },
	/* 0xAB */{ ClearBit<5, E>::execute, "RES 5 E", 8 },
	/* 0xAC */{ ClearBit<5, H>::execute, "RES 5 H", 8 },
	/* 0xAD */{ ClearBit<5, L>::execute, "RES 5 L", 8 },
	/* 0xAE */{ ClearBit<5, PtrHL>::execute, "RES 5 (HL)", 12 },
	/* 0xAF */{ ClearBit<5, A>::execute, "RES 5 A", 8 },

	/* 0xB0 */{ ClearBit<6, B>::execute, "RES 6 B", 8 },
	/* 0xB1 */{ ClearBit<6, C>::execute, "RES 6 C", 8 },
	/* 0xB2 */{ ClearBit<6, D>::execute, "RES 6 D", 8 },
	/* 0xB3 */{ ClearBit<6, E>::execute, "RES 6 E", 8 },
	/* 0xB4 */{ ClearBit<6, H>::execute, "RES 6 H", 8 },
	/* 0xB5 */{ ClearBit<6, L>::execute, "RES 6 L", 8 },
	/* 0xB6 */{ ClearBit<6, PtrHL>::execute, "RES 6 (HL)", 16 },
	/* 0xB7 */{ ClearBit<6, A>::execute, "RES 6 A", 8 },
	/* 0xB8 */{ ClearBit<7, B>::execute, "RES 7 B", 8 },
	/* 0xB9 */{ ClearBit<7, C>::execute, "RES 7 C", 8 },
	/* 0xBA */{ ClearBit<7, D>::execute, "RES 7 D", 8 },
	/* 0xBB */{ ClearBit<7, E>::execute, "RES 7 E", 8 },
	/* 0xBC */{ ClearBit<7, H>::execute, "RES 7 H", 8 },
	/* 0xBD */{ ClearBit<7, L>::execute, "RES 7 L", 8 },
	/* 0xBE */{ ClearBit<7, PtrHL>::execute, "RES 7 (HL)", 12 },
	/* 0xBF */{ ClearBit<7, A>::execute, "RES 7 A", 8 },

	/* 0xC0 */{ SetBit<0, B>::execute, "SET 0 B", 8 },
	/* 0xC1 */{ SetBit<0, C>::execute, "SET 0 C", 8 },
	/* 0xC2 */{ SetBit<0, D>::execute, "SET 0 D", 8 },
	/* 0xC3 */{ SetBit<0, E>::execute, "SET 0 E", 8 },
	/* 0xC4 */{ SetBit<0, H>::execute, "SET 0 H", 8 },
	/* 0xC5 */{ SetBit<0, L>::execute, "SET 0 L", 8 },
	/* 0xC6 */{ SetBit<0, PtrHL>::execute, "SET 0 (HL)", 16 },
	/* 0xC7 */{ SetBit<0, A>::execute, "SET 0 A", 8 },
	/* 0xC8 */{ SetBit<1, B>::execute, "SET 1 B", 8 },
	/* 0xC9 */{ SetBit<1, C>::execute, "SET 1 C", 8 },
	/* 0xCA */{ SetBit<1, D>::execute, "SET 1 D", 8 },
	/* 0xCB */{ SetBit<1, E>::execute, "SET 1 E", 8 },
	/* 0xCC */{ SetBit<1, H>::execute, "SET 1 H", 8 },
	/* 0xCD */{ SetBit<1, L>::execute, "SET 1 L", 8 },
	/* 0xCE */{ SetBit<1, PtrHL>::execute, "SET 1 (HL)", 12 },
	/* 0xCF */{ SetBit<1, A>::execute, "SET 1 A", 8 },

	/* 0xD0 */{ SetBit<2, B>::execute, "SET 2 B", 8 },
	/* 0xD1 */{ SetBit<2, C>::execute, "SET 2 C", 8 },
	/* 0xD2 */{ SetBit<2, D>::execute, "SET 2 D", 8 },
	/* 0xD3 */{ SetBit<2, E>::execute, "SET 2 E", 8 },
	/* 0xD4 */{ SetBit<2, H>::execute, "SET 2 H", 8 },
	/* 0xD5 */{ SetBit<2, L>::execute, "SET 2 L", 8 },
	/* 0xD6 */{ SetBit<2, PtrHL>::execute, "SET 2 (HL)", 16 },
	/* 0xD7 */{ SetBit<2, A>::execute, "SET 2 A", 8 },
	/* 0xD8 */{ SetBit<3, B>::execute, "SET 3 B", 8 },
	/* 0xD9 */{ SetBit<3, C>::execute, "SET 3 C", 8 },
	/* 0xDA */{ SetBit<3, D>::execute, "SET 3 D", 8 },
	/* 0xDB */{ SetBit<3, E>::execute, "SET 3 E", 8 },
	/* 0xDC */{ SetBit<3, H>::execute, "SET 3 H", 8 },
	/* 0xDD */{ SetBit<3, L>::execute, "SET 3 L", 8 },
	/* 0xDE */{ SetBit<3, PtrHL>::execute, "SET 3 (HL)", 12 },
	/* 0xDF */{ SetBit<3, A>::execute, "SET 3 A", 8 },

	/* 0xE0 */{ SetBit<4, B>::execute, "SET 4 B", 8 },
	/* 0xE1 */{ SetBit<4, C>::execute, "SET 4 C", 8 },
	/* 0xE2 */{ SetBit<4, D>::execute, "SET 4 D", 8 },
	/* 0xE3 */{ SetBit<4, E>::execute, "SET 4 E", 8 },
	/* 0xE4 */{ SetBit<4, H>::execute, "SET 4 H", 8 },
	/* 0xE5 */{ SetBit<4, L>::execute, "SET 4 L", 8 },
	/* 0xE6 */{ SetBit<4, PtrHL>::execute, "SET 4 (HL)", 16 },
	/* 0xE7 */{ SetBit<4, A>::execute, "SET 4 A", 8 },
	/* 0xE8 */{ SetBit<5, B>::execute, "SET 5 B", 8 },
	/* 0xE9 */{ SetBit<5, C>::execute, "SET 5 C", 8 },
	/* 0xEA */{ SetBit<5, D>::execute, "SET 5 D", 8 },
	/* 0xEB */{ SetBit<5, E>::execute, "SET 5 E", 8 },
	/* 0xEC */{ SetBit<5, H>::execute, "SET 5 H", 8 },
	/* 0xED */{ SetBit<5, L>::execute, "SET 5 L", 8 },
	/* 0xEE */{ SetBit<5, PtrHL>::execute, "SET 5 (HL)", 12 },
	/* 0xEF */{ SetBit<5, A>::execute, "SET 5 A", 8 },

	/* 0xF0 */{ SetBit<6, B>::execute, "SET 6 B", 8 },
	/* 0xF1 */{ SetBit<6, C>::execute, "SET 6 C", 8 },
	/* 0xF2 */{ SetBit<6, D>::execute, "SET 6 D", 8 },
	/* 0xF3 */{ SetBit<6, E>::execute, "SET 6 E", 8 },
	/* 0xF4 */{ SetBit<6, H>::execute, "SET 6 H", 8 },
	/* 0xF5 */{ SetBit<6, L>::execute, "SET 6 L", 8 },
	/* 0xF6 */{ SetBit<6, PtrHL>::execute, "SET 6 (HL)", 16 },
	/* 0xF7 */{ SetBit<6, A>::execute, "SET 6 A", 8 },
	/* 0xF8 */{ SetBit<7, B>::execute, "SET 7 B", 8 },
	/* 0xF9 */{ SetBit<7, C>::execute, "SET 7 C", 8 },
	/* 0xFA */{ SetBit<7, D>::execute, "SET 7 D", 8 },
	/* 0xFB */{ SetBit<7, E>::execute, "SET 7 E", 8 },
	/* 0xFC */{ SetBit<7, H>::execute, "SET 7 H", 8 },
	/* 0xFD */{ SetBit<7, L>::execute, "SET 7 L", 8 },
	/* 0xFE */{ SetBit<7, PtrHL>::execute, "SET 7 (HL)", 12 },
	/* 0xFF */{ SetBit<7, A>::execute, "SET 7 A", 8 },
};

void State::process()
{
	auto pc = this->cpu.pc;
	this->cpu.pc++;
	auto op = this->get_memory_byte(pc);
	auto instruction = &instructions[op];

	if (instruction->op == nullptr) {
		std::stringstream stream;
		stream << "Operation \"" << instruction->name << "\" is not implemented";
		this->panic(stream.str());
	}
	this->log_op(pc, instruction->name);
	instruction->op(*this);
	this->cycles += instruction->cycles;
}

void State::process_cb() {
	auto pc = this->cpu.pc;
	this->cpu.pc++;
	auto op = this->get_memory_byte(pc);

	auto instruction = &extended_instructions[op];
	if (instruction->op == nullptr) {
		std::stringstream stream;
		stream << "Extended operation \"" << instruction->name << "\" is not implemented";
		this->panic(stream.str());
	}

	this->log_op(pc, instruction->name);
	instruction->op(*this);
	this->cycles += instruction->cycles;
}

std::ostream &operator<<(std::ostream &os, State const &state)
{
	return os
		<< "Cpu { "
		<< state.cpu
		<< " }";
}
