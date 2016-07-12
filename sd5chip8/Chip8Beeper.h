#pragma once

#include "Chip8Types.h"
#include "Chip8Constants.h"

#include <memory>

#include <SFML\Audio\SoundBuffer.hpp>
#include <SFML\Audio\Sound.hpp>

/**
* Handles the emulation of Chip-8 sounds.
*/
class Chip8Beeper
{
public:
	Chip8Beeper(
		unsigned int samples = CHIP8_BEEPER_DEFAULT_SAMPLES, 
		unsigned int sampleRate = CHIP8_BEEPER_DEFAULT_SAMPLE_RATE, 
		unsigned int amplitude = CHIP8_BEEPER_DEFAULT_AMPLITUDE
		);
	~Chip8Beeper();

	/**
	* Turns beeping on or off.
	*/
	void SetBeeping(bool val);

	/**
	* Returns whether or not the beeper is currently beeping.
	*/
	bool GetBeeping() const;

	/**
	* Gets the amount of samples used in the beep.
	*/
	unsigned int GetSampleAmount() const;

	/**
	* Gets the sample rate of the beep.
	*/
	unsigned int GetSampleRate() const;

	/**
	* Gets the amplitude of the beep.
	*/
	unsigned int GetAmplitude() const;

private:
	const unsigned int samples_, sampleRate_, amplitude_;

	sf::SoundBuffer beepBuf_;
	sf::Sound beep_;
	bool isBeeping_;

	/**
	* Initializes the beep sound.
	*/
	bool InitializeBeep();
};

