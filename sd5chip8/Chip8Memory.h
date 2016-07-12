#pragma once

#include <memory>

#include "Chip8Constants.h"
#include "Chip8Types.h"

/**
* Represents the RAM used by a Chip-8 program.
*/
class Chip8Memory
{
public:
	/**
	* Allocates Chip-8 program RAM of a specified size (in bytes).
	*/
	Chip8Memory(u16 size = CHIP8_MEMORY_SIZE);
	~Chip8Memory();

	/**
	* Zero's the RAM.
	*/
	void Reset();

	/**
	* Writes the value of a variable in memory at the specified address to outVal if outVal is not null.
	* If the memory could be read, true is returned. False otherwise.
	*/
	bool ReadValue(u16 address, u8* outVal);

	/**
	* Writes a value to a variable in memory at the specified address.
	* If the memory could be written to, true is returned. False otherwise.
	*/
	bool WriteValue(u16 address, u8 val);

	/**
	* Gets the current amount of allocated Chip-8 RAM in bytes.
	*/
	u16 GetAllocatedSize() const;

private:
	const u16 memSize_;
	std::unique_ptr<u8[]> mem_;
};

