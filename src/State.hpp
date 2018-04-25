#pragma once
#include <cstdint>
#include <ostream>
#include <array>
#include "Cpu.hpp"
#include "Rom.hpp"

using Memory = std::array<uint8_t, 8 * 1024>;
using Video = std::array<uint8_t, 8 * 1024>;

class State
{
public:
	State();

	Cpu cpu;
	Rom rom;
	Memory memory;
	Video video;
	bool is_error;
	uint8_t interrupt_enabled;

	[[noreturn]]
	void panic(const std::string&) const;
	uint8_t consume_byte();
	uint8_t get_memory_byte(uint16_t addr) const;
	void set_memory_byte(uint16_t addr, uint8_t value);
	void process();
};

std::ostream &operator<<(std::ostream &os, State const &state);
