#include "Chip8Beeper.h"


Chip8Beeper::Chip8Beeper(unsigned int samples, unsigned int sampleRate, unsigned int amplitude) :
isBeeping_(false),
samples_(samples),
sampleRate_(sampleRate),
amplitude_(amplitude)
{
	beep_.setLoop(true);
	InitializeBeep();
}


Chip8Beeper::~Chip8Beeper()
{
}


bool Chip8Beeper::InitializeBeep()
{
	// Construct the sine wave data to use for beeping.
	auto soundData = std::unique_ptr<sf::Int16[]>(new sf::Int16[samples_]);
	for (size_t i = 0; i < samples_; ++i)
	{
		soundData[i] = static_cast<sf::Int16>(amplitude_ * sinf((i * 0.01f) * 6.28f));
	}

	// Store data in sound buffer.
	if (!beepBuf_.loadFromSamples(soundData.get(), samples_, 1, sampleRate_))
	{
		// Failed to load sound!
		return false;
	}

	beep_.setBuffer(beepBuf_);
	return true;
}


void Chip8Beeper::SetBeeping(bool val)
{
	if (isBeeping_ == val)
	{
		return;
	}

	isBeeping_ = val;
	(isBeeping_ ? beep_.play() : beep_.stop());
}


bool Chip8Beeper::GetBeeping() const
{
	return isBeeping_;
}


unsigned int Chip8Beeper::GetSampleAmount() const
{
	return samples_;
}


unsigned int Chip8Beeper::GetSampleRate() const
{
	return sampleRate_;
}


unsigned int Chip8Beeper::GetAmplitude() const
{
	return amplitude_;
}