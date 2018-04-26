#include "IO.hpp"
#include <iostream>


uint8_t * IO::get_field(uint8_t addr)
{
	switch (addr) {
	case 0x11: return &this->SoundModule1Pattern;
	case 0x12: return &this->SoundModule1Envelope;
	case 0x24: return &this->ChannelVolume;
	case 0x25: return &this->SoundOutputTerminal;
	case 0x26: return &this->SoundOnOff;
	case 0x40: return reinterpret_cast<uint8_t*>(&this->LCDControl);
	case 0x42: return &this->ScrollY;
	case 0x44: return &this->VerticalLine;
	case 0x47: return &this->BackgroundWindowPaletteData;
	default:
		std::cout << "Could not find field at IO address 0x" << std::hex << static_cast<uint32_t>(addr) << std::endl;
		return nullptr;
	}
}

uint8_t IO::get(uint8_t addr)
{
	auto address = this->get_field(addr);
	if (address == nullptr) {
		return 0;
	}
	else {
		return *address;
	}
}

void IO::set(uint8_t addr, uint8_t value)
{
	auto address = this->get_field(addr);
	if (address != nullptr) {
		*address = value;
	}
}
