#include <cstdlib>
#include <iostream>

#include <SFML\Graphics\RenderWindow.hpp>
#include <SFML\Window\Event.hpp>

#include "Chip8.h"


/**
* Main entry point for program.
*/
int main(int argc, char* argv[])
{
#ifdef CHIP8_RELEASE
	std::cout << "SD5 Chip-8 [Release]";
#elif CHIP8_DEBUG
	std::cout << "SD5 Chip-8 [Debug]";
#else
	std::cout << "SD5 Chip-8";
#endif
	std::cout << std::endl << std::endl;

	// Ask for program name as input.
	std::cout << "Specify program to load: ";
	std::string programFileName;
	getline(std::cin, programFileName);

	// Ask if program is for the ETI 660.
	std::cout << "Is this an ETI 660 program? (Y / N): ";
	const auto isETI660YN = tolower(getchar());
	std::cout << std::endl;

	// Create window and Chip-8 emu instance.
	auto window = sf::RenderWindow(
		sf::VideoMode(CHIP8_WINDOW_WIDTH, CHIP8_WINDOW_HEIGHT),
		"SD5 Chip-8"
		);

	// Create default font.
	sf::Font font;
	const auto isFontLoaded = font.loadFromFile(CHIP8_EMULATOR_DEFAULT_FONT_FILENAME);
	if (!isFontLoaded)
	{
		std::cerr << "Warning: Failed to load emulator font!" << std::endl;
	}

	// Create emulator instance and feed in font if it successfully loaded.
	Chip8 chip8(window, (isFontLoaded ? &font : nullptr));
#ifdef CHIP8_DEBUG
	chip8.SetDebugMode(true);
#endif

	// Attempt to load program.
	if (!chip8.LoadProgram(programFileName, (isETI660YN == 'y')))
	{
		// Failed to load program.
		std::cerr << "Program load error - exiting." << std::endl;
		return EXIT_FAILURE;
	}

	std::cout << "Running program..." << std::endl;
	while (window.isOpen())
	{
		// Handle window message queue events.
		sf::Event event;
		while (window.pollEvent(event))
		{
			switch (event.type)
			{
			// Handle window close event by closing the window.
			case sf::Event::Closed:
				window.close();
				break;

			// Handle window key press.
			case sf::Event::KeyPressed:
				// F11 for soft reset.
				if (event.key.code == sf::Keyboard::F11)
				{
					chip8.SoftReset();
				}
				// F1 for debug toggle.
				else if (event.key.code == sf::Keyboard::F1)
				{
					chip8.SetDebugMode(!chip8.IsInDebugMode());
				}
				break;
			}
		}

		if (!chip8.RunFrame())
		{
			// Program error.
			std::cerr << "Program execution error - exiting." << std::endl;
			return EXIT_FAILURE;
		}

		// Display whats been drawn to the screen.
		window.display();
	}

	std::cout << "Window closed - exiting." << std::endl;
	return EXIT_SUCCESS;
}