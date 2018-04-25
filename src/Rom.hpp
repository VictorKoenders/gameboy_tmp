#pragma once
#include <vector>
#include <cstdint>

enum class RomSize {
	_32kbyte,
	_64kbyte,
	_128kbyte,
	_256kbyte,
	_512kbyte,
	_1mbyte,
	_2mbyte,
	_1_1mbyte,
	_1_2mbyte,
	_1_5mbyte
};

enum class RamSize {
	none,
	_2kb,
	_8kb,
	_32kb,
	_128kb
};

class Rom
{
	std::vector<uint8_t> buffer;
	bool validate_rom();
	RomSize rom_size;
	RamSize ram_size;
public:
	Rom();
	bool load(const char* file);
	size_t len() const;
	uint8_t get(uint16_t addr) const;
	~Rom();
};
