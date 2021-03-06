#include "Cpu.hpp"

Flags operator |(const Flags lhs, const Flags rhs)
{
	return static_cast<Flags> (
		static_cast<std::underlying_type<Flags>::type>(lhs) |
		static_cast<std::underlying_type<Flags>::type>(rhs)
	);
}

Flags operator|=(Flags & lhs, const Flags rhs)
{
	lhs = static_cast<Flags> (
		static_cast<std::underlying_type<Flags>::type>(lhs) |
		static_cast<std::underlying_type<Flags>::type>(rhs)
	);

	return lhs;
}

Flags operator &(const Flags lhs, const Flags rhs)
{
	return static_cast<Flags> (
		static_cast<std::underlying_type<Flags>::type>(lhs) &
		static_cast<std::underlying_type<Flags>::type>(rhs)
	);
}

std::ostream &operator<<(std::ostream &os, Cpu const &cpu)
{
	return os
		<< "A: " << static_cast<uint32_t>(cpu.A) /* << " / 0x" << std::uppercase << std::hex << static_cast<uint32_t>(cpu.A) */ << ", "
		<< "B: " << static_cast<uint32_t>(cpu.B) /* << " / 0x" << std::uppercase << std::hex << static_cast<uint32_t>(cpu.B) */ << ", "
		<< "C: " << static_cast<uint32_t>(cpu.C) /* << " / 0x" << std::uppercase << std::hex << static_cast<uint32_t>(cpu.C) */ << ", "
		<< "D: " << static_cast<uint32_t>(cpu.D) /* << " / 0x" << std::uppercase << std::hex << static_cast<uint32_t>(cpu.D) */ << ", "
		<< "E: " << static_cast<uint32_t>(cpu.E) /* << " / 0x" << std::uppercase << std::hex << static_cast<uint32_t>(cpu.E) */ << ", "
		<< "F: " << static_cast<uint32_t>(cpu.F) /* << " / 0x" << std::uppercase << std::hex << static_cast<uint32_t>(cpu.F) */ << ", "
		<< "H: " << static_cast<uint32_t>(cpu.H) /* << " / 0x" << std::uppercase << std::hex << static_cast<uint32_t>(cpu.H) */ << ", "
		<< "L: " << static_cast<uint32_t>(cpu.L) /* << " / 0x" << std::uppercase << std::hex << static_cast<uint32_t>(cpu.L) */ << "";
}

Cpu::Cpu() :
	A(0),
	B(0),
	C(0),
	D(0),
	E(0),
	F(0),
	H(0),
	L(0),
	sp(0),
	pc(0x0),
	flags(Flags::Empty)
{

}