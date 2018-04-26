#pragma once
#include <cstdint>
#include "enum_utils.hpp"

enum class LCDControlFlags: uint8_t {
	/// Bit 7 - LCD Control Operation *
	/// 	0: Stop completely(no picture on screen)
	/// 	1 : operation
	Enabled	= 0b10000000,

	/// Bit 6 - Window Tile Map Display Select
	/// 	0 : $9800 - $9BFF
	/// 	1 : $9C00 - $9FFF
	WindowTileMapHighMemory = 0b01000000,

	/// Bit 5 - Window Display
	/// 	0 : off
	/// 	1 : on
	WindowDisplay = 0b00100000,

	/// Bit 4 - BG & Window Tile Data Select
	/// 	0 : $8800 - $97FF
	/// 	1 : $8000 - $8FFF <-Same area as OBJ
	TileDataSelect = 0b00010000,

	/// Bit 3 - BG Tile Map Display Select
	/// 	0 : $9800 - $9BFF
	/// 	1 : $9C00 - $9FFF
	BGTileMapHighMemory = 0b00001000,

	/// Bit 2 - OBJ(Sprite) Size
	/// 	0 : 8 * 8
	/// 	1 : 8 * 16 (width*height)
	SpriteSize = 0b00000100,

	/// Bit 1 - OBJ(Sprite) Display
	/// 	0 : off
	/// 	1 : on
	SpriteVisible = 0b00000010,

	/// Bit 0 - BG & Window Display
	/// 	0 : off
	/// 	1 : on
	WindowVisible = 0b00000001
};
ENABLE_BITMASK_OPERATORS(LCDControlFlags);

class IO
{
	uint8_t* get_field(uint8_t addr);
public:
	uint8_t SoundModule1Pattern = 0;
	uint8_t SoundModule1Envelope = 0;
	uint8_t ChannelVolume = 0;
	uint8_t SoundOutputTerminal = 0;
	uint8_t SoundOnOff = 0xF1;
	/// <summary>LCD control register (0xFF40)</summary>
	LCDControlFlags LCDControl = LCDControlFlags::WindowTileMapHighMemory | LCDControlFlags::TileDataSelect | 
								 LCDControlFlags::BGTileMapHighMemory | LCDControlFlags::SpriteVisible | LCDControlFlags::WindowVisible;
	uint8_t ScrollY = 0;

	// The current position of the vertical sync of the screen
	uint8_t VerticalLine = 0x90;
	uint8_t BackgroundWindowPaletteData = 0;

	uint8_t get(uint8_t addr);
	void set(uint8_t addr, uint8_t value);
};

