#pragma once

#include "Chip8Types.h"

#include <utility>

#include <SFML\Window\Keyboard.hpp>

/**
* Contains methods and data needed to emulate the Chip-8 keyboard.
*/
namespace Chip8Keyboard
{
	/**
	* Contains a collection of SFML keys whose index values match their corrisponding Chip-8 key code.
	*/
	const sf::Keyboard::Key keys[16] = {
		sf::Keyboard::X,	// 0
		sf::Keyboard::Num1, // 1
		sf::Keyboard::Num2,	// 2
		sf::Keyboard::Num3, // 3
		sf::Keyboard::Q,	// 4
		sf::Keyboard::W,	// 5
		sf::Keyboard::E,	// 6
		sf::Keyboard::A,	// 7
		sf::Keyboard::S,	// 8
		sf::Keyboard::D,	// 9
		sf::Keyboard::Z,	// A
		sf::Keyboard::C,	// B
		sf::Keyboard::Num4,	// C
		sf::Keyboard::R,	// D
		sf::Keyboard::F,	// E
		sf::Keyboard::V		// F
	};

	/**
	* Returns whether or not the specified key is currently pressed down.
	*/
	bool isKeyDown(u8 key);

	/**
	* Writes to outKey containing the value of the current pressed key if there is one and returns true.
	* If no key is currently being pressed, false is returned and outKey is not modified.
	*/
	bool getCurrentPressedKey(u8* outKey);
};

