#pragma once
#include <cstdint>
#include <ostream>
#include <array>
#include "Cpu.hpp"
#include "Rom.hpp"
#include "IO.hpp"

using Memory = std::vector<uint8_t>;
using InternalMemory = std::array<uint8_t, 0x7F>;
using Video = std::array<uint8_t, 8 * 1024>;

class State
{
	std::vector<std::array<uint8_t, 0x2000>> banks;
	size_t current_bank;
	bool boot_rom_enabled;
	void log_op(uint16_t, const char*);
public:
	State();

	Cpu cpu;
	Rom rom;
	IO io;
	Memory memory;
	InternalMemory internal_memory;
	Video video;
	bool is_error;
	uint8_t interrupt_enabled;
	uint32_t cycles;

	[[noreturn]]
	void panic(const std::string&) const;
	uint8_t consume_byte();
	uint8_t get_memory_byte(uint16_t addr);
	void set_memory_byte(uint16_t addr, uint8_t value);
	void process();
	void process_cb();
};

std::ostream &operator<<(std::ostream &os, State const &state);
