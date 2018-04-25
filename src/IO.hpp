#pragma once
#include <cstdint>

class IO
{
	uint8_t* get_field(uint8_t addr);
public:
	uint8_t SoundModule1Pattern = 0;
	uint8_t SoundModule1Envelope = 0;
	uint8_t ChannelVolume = 0;
	uint8_t SoundOutputTerminal = 0;
	uint8_t SoundOnOff = 0xF1;
	uint8_t LCDControl = 0;
	uint8_t ScrollY = 0;

	// The current position of the vertical sync of the screen
	uint8_t VerticalLine = 0x90;
	uint8_t BackgroundWindowPaletteData = 0;

	uint8_t get(uint8_t addr);
	void set(uint8_t addr, uint8_t value);
};

