#include "State.hpp"
#include <iostream>
#include <sstream>

State::State() :
	cpu(),
	rom(),
	memory(),
	video(),
	is_error(false),
	interrupt_enabled(0)
{
	if (!this->rom.load("tetris.gb")) {
		this->is_error = true;
	}
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
		stream << "Could not read from 0x" << std::hex << addr << ", unused memory";
		this->panic(stream.str());
	}
	if (addr >= 0xFF00) {
		// I/O
		std::ostringstream stream;
		stream << "Could not read from 0x" << std::hex << addr << ", I/O not implemented";
		this->panic(stream.str());
	}
	if (addr >= 0xFEA0) {
		// Empty but unusable for I/O
		std::ostringstream stream;
		stream << "Could not read from 0x" << std::hex << addr << ", unused memory";
		this->panic(stream.str());
	}
	if (addr >= 0xFE00) {
		// Sprite Attrib Memory
		std::ostringstream stream;
		stream << "Could not read from 0x" << std::hex << addr << ", Sprite Attrib memory not implemented";
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
		stream << "Could not read from 0x" << std::hex << addr << ", switchable RAM bank not implemented";
		this->panic(stream.str());
	}
	if (addr >= 0x8000) {
		// Video Ram
		return this->video[addr - 0x8000];
	}
	if (addr >= 0x4000) {
		// 16kB switchable ROMbank
		std::ostringstream stream;
		stream << "Could not read from 0x" << std::hex << addr << ", switchable ROM bank not implemented";
		this->panic(stream.str());
	}
	// 16kB ROM bank #0
	if (addr < this->rom.len()) {
		return this->rom.get(addr);
	}
	return 0;
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
		stream << "Could not write to 0x" << std::hex << addr << ", unused memory";
		this->panic(stream.str());
	}
	else if (addr >= 0xFF00) {
		// I/O
		std::ostringstream stream;
		stream << "Could not write to 0x" << std::hex << addr << ", I/O not implemented";
		this->panic(stream.str());
	}
	else if (addr >= 0xFEA0) {
		// Empty but unusable for I/O
		std::ostringstream stream;
		stream << "Could write to from 0x" << std::hex << addr << ", unused memory";
		this->panic(stream.str());
	}
	else if (addr >= 0xFE00) {
		// Sprite Attrib Memory
		std::ostringstream stream;
		stream << "Could not write to 0x" << std::hex << addr << ", Sprite Attrib memory not implemented";
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
		stream << "Could not write to 0x" << std::hex << addr << ", switchable RAM bank not implemented";
		this->panic(stream.str());
	}
	else if (addr >= 0x8000) {
		// Video Ram
		this->video[addr - 0x8000] = value;
	}
	else if (addr >= 0x4000) {
		// 16kB switchable ROMbank
		std::ostringstream stream;
		stream << "Could not write to 0x" << std::hex << addr << ", switchable ROM bank not implemented";
		this->panic(stream.str());
	}
	else {
		// 16kB ROM bank #0
		std::ostringstream stream;
		stream << "Could not write to 0x" << std::hex << addr << ", ROM bank not implemented";
		this->panic(stream.str());
	}
}

void State::process()
{
	if (this->is_error) return;

	auto op = this->get_memory_byte(this->cpu.pc);
	switch (op) {
	default:
		std::stringstream stream;
		stream << "Could not process opcode 0x" << std::hex << static_cast<uint32_t>(op);
		this->panic(stream.str());
	}
}

std::ostream &operator<<(std::ostream &os, State const &state)
{
	return os
		<< "Cpu { "
		<< state.cpu
		<< " }"
		<< std::endl
		<< "Error state? "
		<< (state.is_error ? "Yes" : "No");
}
