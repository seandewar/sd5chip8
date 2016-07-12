#include "Chip8Memory.h"

#include <cassert>


Chip8Memory::Chip8Memory(u16 size) :
memSize_(size)
{
	// Allocate program RAM of specified size and zero out the memory.
	mem_ = std::unique_ptr<u8[]>(new u8[size]);
	Reset();
}


Chip8Memory::~Chip8Memory()
{
}


void Chip8Memory::Reset()
{
	for (u16 i = 0; i < memSize_; ++i)
	{
		mem_[i] = 0;
	}
}


bool Chip8Memory::ReadValue(u16 address, u8* outVal)
{
	// Make sure that the address we read from is valid.
	if (address >= memSize_)
	{
		return false;
	}

	if (outVal != nullptr)
	{
		*outVal = mem_[address];
	}
	return true;
}


bool Chip8Memory::WriteValue(u16 address, u8 val)
{
	// Make sure that the address we write to is valid.
	if (address >= memSize_)
	{
		return false;
	}

	mem_[address] = val;
	return true;
}


u16 Chip8Memory::GetAllocatedSize() const
{
	return memSize_;
}