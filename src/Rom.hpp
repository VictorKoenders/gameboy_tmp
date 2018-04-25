#pragma once
#include <vector>
#include <cstdint>

enum class Cartridge {
	RomOnly,
	RomMBC1,
	RomMBC1Ram,
	RomMBC1RamBatt,
	RomMBC2,
	RomMBC2Batt,
	RomRam,
	RomRamBatt,
	RomMMM01,
	RomMMM01SRam,
	RomMMM01SRamBatt,
	RomMBC3TimerBatt,
	RomMBC3TimerRamBatt,
	RomMBC3,

	RomMBC3Ram,
	RomMBC3RamBatt,
	RomMBC5,
	RomMBC5Ram,
	RomMBC5RamBatt,
	RomMBC5Rumble,
	RomMBC5RumbleSRam,
	RomMBC5RumbleSRamBatt,
	PocketCamera,
	BandaiTAMA5,
	HudsonHuC3,
	HudsonHuC1,
};

struct CartridgeInfo {
	size_t memory_size;
	size_t bank_count;

	CartridgeInfo(size_t ms, size_t bc) : memory_size(ms), bank_count(bc) {}
};

CartridgeInfo get_cartridge_info(const Cartridge& cartridge);

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
	RomSize _rom_size;
	RamSize _ram_size;
	Cartridge _cartridge;
public:
	Rom();
	bool load(const char* file);
	size_t len() const;
	uint8_t get(uint16_t addr) const;
	RomSize rom_size() const;
	RamSize ram_size() const;
	Cartridge cartridge() const;
	~Rom();
};
