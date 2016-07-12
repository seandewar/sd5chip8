#include "Chip8.h"

#include <fstream>
#include <iostream>
#include <thread>

#include "Chip8Helper.h"


Chip8::Chip8(sf::RenderTarget& target, const sf::Font* defaultSystemFont) :
target_(target),
defaultFont_(defaultSystemFont),
isInDebugMode_(false)
{
}


Chip8::~Chip8()
{
}


bool Chip8::LoadProgram(const std::string& fileName, bool isETI660Program)
{
	std::cout << "Loading program \"" << fileName << "\", (" << (isETI660Program ? "ETI 660" : "Normal") << ")..." << std::endl;
	cpu_.reset();

	auto file = std::ifstream(fileName, std::ios_base::binary);
	if (!file.is_open())
	{
		// Failed to open file.
		std::cerr << "Failed to load program - could not open file." << std::endl;
		return false;
	}

	// Init RAM.
	ram_ = std::make_unique<Chip8Memory>((isETI660Program ? CHIP8_MEMORY_ETI660_SIZE : CHIP8_MEMORY_SIZE));

	u16 size = 0;
	u16 i = (isETI660Program ? CHIP8_PROGRAM_ETI660_START : CHIP8_PROGRAM_START); // ETI660 programs start at 0x600, not 0x200.
	while (file.good())
	{
		if (!ram_->WriteValue(i, file.get()))
		{
			// Failed to write to memory - program maybe too big?
			std::cerr << "Failed to load program - failed to copy program into memory, is the file too large?" << std::endl;
			return false;
		}
		++i;
		++size;
	}

	if (!file.eof())
	{
		// An IO error occurred.
		std::cerr << "Failed to load program - IO error while reading file." << std::endl;
		return false;
	}

	// Init CPU so that it is ready for the program.
	std::cout << "Program load successful! (Size: " << size << "B)" << std::endl;
	cpu_ = std::make_unique<Chip8CPU>(*ram_.get(), display_, &beeper_, isETI660Program);
	return true;
}


bool Chip8::RunFrame()
{
	const auto nextSleepTime = std::chrono::high_resolution_clock::time_point(
		Chip8Helper::GetNowDuration() + std::chrono::microseconds(CHIP8_FRAME_SLEEP_MICROSECONDS));

	if (cpu_ == nullptr)
	{
		// CPU not active - no loaded program.
		target_.clear(sf::Color(0, 0, 0));
		return true;
	}

	// Run one CPU frame.
	const auto cpuFrameResult = cpu_->RunFrame();

	// Render the display.
	display_.Render(target_);

	// Check if debug mode is on and that we have a valid font.
	if (isInDebugMode_ && defaultFont_ != nullptr)
	{
		cpu_->RenderCPUDebug(target_, *defaultFont_);
	}

	// If execution error, return false.
	if (!cpuFrameResult)
	{
		return false;
	}

	std::this_thread::sleep_until(nextSleepTime);
	return true;
}


bool Chip8::SoftReset()
{
	std::cout << "Performing soft reset..." << std::endl;

	if (cpu_ == nullptr)
	{
		// CPU not active, cannot reset.
		std::cerr << "Cannot soft reset - no CPU active!" << std::endl;
		return false;
	}

	cpu_->Reset();
	return true;
}


void Chip8::SetDebugMode(bool val)
{
	isInDebugMode_ = val;
}


bool Chip8::IsInDebugMode() const
{
	return isInDebugMode_;
}