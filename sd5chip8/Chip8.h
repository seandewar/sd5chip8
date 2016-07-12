#pragma once

#include <string>
#include <memory>
#include <chrono>

#include <SFML\Graphics\RenderTarget.hpp>
#include <SFML\Graphics\Font.hpp>

#include "Chip8Constants.h"
#include "Chip8CPU.h"
#include "Chip8Beeper.h"

/**
* The main chip8 class.
*/
class Chip8
{
public:
	/**
	* Create a new instance of the emulator hooked to a RenderTarget target.
	* A font can be specified which will be used by the emulator for debug info .etc (optional - pass null if not needed)
	*/
	Chip8(sf::RenderTarget& target, const sf::Font* defaultSystemFont = nullptr);
	~Chip8();

	/**
	* Loads a Chip-8 program into memory.
	* Returns true on success, false on failure.
	*/
	bool LoadProgram(const std::string& fileName, bool isETI660Program = false);

	/**
	* Runs the loaded program for one frame.
	* Clears the screen if no program is loaded or CPU isn't active and returns true anyway.
	* Returns true on success, false on failure.
	*/
	bool RunFrame();

	/**
	* Performs a soft reset.
	* Returns true on success, false on failure.
	*/
	bool SoftReset();

	/**
	* Sets debug mode on or off.
	*/
	void SetDebugMode(bool val);

	/**
	* Gets whether or not debug mode is active.
	*/
	bool IsInDebugMode() const;

private:
	const sf::Font* defaultFont_;
	sf::RenderTarget& target_;

	std::unique_ptr<Chip8CPU> cpu_;
	std::unique_ptr<Chip8Memory> ram_;
	Chip8Display display_;
	Chip8Beeper beeper_;

	bool isInDebugMode_;
};

