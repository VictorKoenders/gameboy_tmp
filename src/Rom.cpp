#include "Rom.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <string>

Rom::Rom() : buffer() {

}

bool Rom::load(const char* file_name)
{
	std::basic_ifstream<uint8_t> f;
	std::ios_base::iostate exceptionMask = f.exceptions() | std::ios::failbit;
	f.exceptions(exceptionMask);

	try {
		f.open(file_name, std::ios::binary);

		f.seekg(std::ios::end);
		this->buffer.reserve(f.tellg());
		f.seekg(std::ios::beg);

		this->buffer.assign(
			std::istreambuf_iterator<uint8_t>(f),
			std::istreambuf_iterator<uint8_t>()
		);
	}
	catch (std::ios_base::failure& e) {
		std::cerr << e.what() << '\n';
		return false;
	}
	return this->validate_rom();
}

size_t Rom::len() const {
	return this->buffer.size();
}

uint8_t Rom::get(uint16_t addr) const {
	return this->buffer[addr];
}

bool Rom::validate_rom() {
	auto name = std::string(reinterpret_cast<const char*>(&this->buffer[0x134]), 8);
	std::cout << "Game name: " << name << std::endl;

	const size_t expected_size = 48;
	const uint8_t expected[expected_size] = {
		0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
		0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
		0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E
	};

	for (int i = 0; i < expected_size; i++) {
		auto actual = this->buffer[0x0104 + i];
		if (expected[i] != actual) {
			std::cout << "Byte at position 0x" << std::hex << static_cast<uint32_t>(0104 + i) << " does not match" << std::endl;
			std::cout << "Expected 0x" << std::hex << static_cast<uint32_t>(expected[i]) << ", got 0x" << std::hex << static_cast<uint32_t>(actual) << std::endl;
			return false;
		}
	}

	std::cout << "Nintento graphic check: Ok" << std::endl;

	std::cout << "Cartridge type: ";
	auto cartridge_type = this->buffer[0x0147];
	switch (cartridge_type) {
	case 0x00: std::cout << "ROM ONLY"; break;
	case 0x01: std::cout << "ROM + MBC1"; break;
	case 0x02: std::cout << "ROM + MBC1 + RAM"; break;
	case 0x03: std::cout << "ROM + MBC1 + RAM + BATT"; break;
	case 0x05: std::cout << "ROM + MBC2"; break;
	case 0x06: std::cout << "ROM + MBC2 + BATTERY"; break;
	case 0x08: std::cout << "ROM + RAM"; break;
	case 0x09: std::cout << "ROM + RAM + BATTERY"; break;
	case 0x0B: std::cout << "ROM + MMM01"; break;
	case 0x0C: std::cout << "ROM + MMM01 + SRAM"; break;
	case 0x0D: std::cout << "ROM + MMM01 + SRAM + BATT"; break;
	case 0x12: std::cout << "ROM + MBC3 + RAM"; break;
	case 0x13: std::cout << "ROM + MBC3 + RAM + BATT"; break;
	case 0x19: std::cout << "ROM + MBC5"; break;
	case 0x1A: std::cout << "ROM + MBC5 + RAM"; break;
	case 0x1B: std::cout << "ROM + MBC5 + RAM + BATT"; break;
	case 0x1C: std::cout << "ROM + MBC5 + RUMBLE"; break;
	case 0x1D: std::cout << "ROM + MBC5 + RUMBLE + SRAM"; break;
	case 0x1E: std::cout << "ROM + MBC5 + RUMBLE + SRAM + BATT"; break;
	case 0x1F: std::cout << "Pocket Camera"; break;
	case 0xFD: std::cout << "Bandai TAMA5"; break;
	case 0xFE: std::cout << "Hudson HuC-3"; break;
	case 0xFF: std::cout << "Hudson HuC-1"; break;
	default:
		std::cout << "Unknown (0x" << std::hex << static_cast<uint32_t>(cartridge_type) << ")";
		return false;
	}
	std::cout << std::endl;
	if (cartridge_type != 0x00) {
		std::cout << "[WARNING] Cartridge type not implemented. Your ROM will probably not work" << std::endl;
	}

	auto rom_size = this->buffer[0x0148];
	std::cout << "ROM size: ";
	switch (rom_size) {
	case 0: std::cout << "32KByte (2 banks)"; this->rom_size = RomSize::_32kbyte; break;
	case 1: std::cout << "64KByte (4 banks)"; this->rom_size = RomSize::_64kbyte; break;
	case 2: std::cout << "128KByte (8 banks)"; this->rom_size = RomSize::_128kbyte; break;
	case 3: std::cout << "256KByte (16 banks)"; this->rom_size = RomSize::_256kbyte; break;
	case 4: std::cout << "512KByte (32 banks)"; this->rom_size = RomSize::_512kbyte; break;
	case 5: std::cout << "1MByte (64 banks)"; this->rom_size = RomSize::_1mbyte; break;
	case 6: std::cout << "2MByte (128 banks)"; this->rom_size = RomSize::_2mbyte; break;
	case 52: std::cout << "1.1MByte (72 banks)"; this->rom_size = RomSize::_1_1mbyte; break;
	case 53: std::cout << "1.2MByte (80 banks)"; this->rom_size = RomSize::_1_2mbyte; break;
	case 54: std::cout << "1.5MByte (96 banks)"; this->rom_size = RomSize::_1_5mbyte; break;
	default:
		std::cout << "Unknown (" << static_cast<uint32_t>(rom_size) << ")";
		return false;
	}
	std::cout << std::endl;

	auto ram_size = this->buffer[0x0149];
	std::cout << "RAM size: ";
	switch (ram_size) {
	case 0: std::cout << "None"; this->ram_size = RamSize::none; break;
	case 1: std::cout << "2kB (1 bank)"; this->ram_size = RamSize::_2kb; break;
	case 2: std::cout << "8kB (1 bank)"; this->ram_size = RamSize::_8kb; break;
	case 3: std::cout << "32kB (8 banks)"; this->ram_size = RamSize::_32kb; break;
	case 4: std::cout << "128kB (16 banks)"; this->ram_size = RamSize::_128kb; break;
	default: std::cout << "Unknown (" << static_cast<uint32_t>(ram_size) << ")"; break;
	}
	std::cout << std::endl;

}

Rom::~Rom()
{
}
