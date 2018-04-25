#pragma once
#include <cstdint>
#include <ostream>

enum class Flags : uint8_t {
	Zero		= 0b10000000,
	Subtract	= 0b01000000,
	HalfCarry	= 0b00100000,
	Carry		= 0b00010000,
	Empty		= 0b00000000,
};

Flags operator |(const Flags lhs, const Flags rhs);
Flags operator |=(Flags &lhs, const Flags rhs);
Flags operator &(const Flags lhs, const Flags rhs);

struct Cpu {
	union {
		struct {
			uint8_t F;
			uint8_t A;
		};
		uint16_t AF;
	};
	union {
		struct
		{
			uint8_t C;
			uint8_t B;
		};
		uint16_t BC;
	};
	union {
		struct
		{
			uint8_t E;
			uint8_t D;
		};
		uint16_t DE;
	};
	union {
		struct
		{
			uint8_t L;
			uint8_t H;
		};
		uint16_t HL;
	};

	uint16_t sp;
	uint16_t pc;
	Flags flags;

	Cpu();
};
std::ostream &operator<<(std::ostream &os, Cpu const &cpu);
