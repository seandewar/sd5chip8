#include "Chip8Keyboard.h"


bool Chip8Keyboard::isKeyDown(u8 key)
{
	if (key >= 16)
	{
		// No such key.
		return false;
	}

	return sf::Keyboard::isKeyPressed(keys[key]);
}


bool Chip8Keyboard::getCurrentPressedKey(u8* outKey)
{
	for (int i = 0; i < 16; ++i)
	{
		if (sf::Keyboard::isKeyPressed(keys[i]))
		{
			if (outKey != nullptr)
			{
				// Write the key code to outKey.
				*outKey = i;
			}

			return true;
		}
	}

	// No keys are currently being pressed.
	return false;
}