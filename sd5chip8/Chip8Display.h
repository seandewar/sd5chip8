#pragma once

#include "Chip8Constants.h"
#include "Chip8Types.h"

#include <SFML\Graphics\RenderTarget.hpp>
#include <SFML\Graphics\Color.hpp>

#include <memory>

/**
* Represents the screen in use by a Chip-8 program.
*/
class Chip8Display
{
public:
	Chip8Display(
		const sf::Color& displayColor = sf::Color(255, 255, 255),
		const sf::Color& backColor = sf::Color(0, 0, 0),
		u8 w = CHIP8_DISPLAY_WIDTH,
		u8 h = CHIP8_DISPLAY_HEIGHT
		);
	~Chip8Display();

	/**
	*  Changes the size of the display and clears it.
	*/
	void Reset(u8 w, u8 h);

	/**
	* Clears the display.
	*/
	void Clear();

	/**
	* Plots a pixel onto the display at co-ords (x, y).
	* If x or y is higher than the width or height of the display,
	* the pixel will be drawn on the opposite side of the screen.
	*/
	void Plot(u16 x, u16 y);

	/**
	* Returns the state of the pixel at co-ords (x, y)
	*/
	u8 GetPixelState(u16 x, u16 y);

	/**
	* Renders the display to a render target.
	*/
	void Render(sf::RenderTarget& target);

	/**
	* Set the color of the display foreground.
	*/
	void SetDisplayColor(const sf::Color& color);

	/**
	* Gets the current color of the display foreground.
	*/
	sf::Color GetDisplayColor() const;

	/**
	* Set the color of the display background.
	*/
	void SetBackgroundColor(const sf::Color& color);

	/**
	* Get the current color of the display background.
	*/
	sf::Color GetBackgroundColor() const;

	/**
	* Get the width of the display in pixels.
	*/
	u8 GetWidth() const;

	/**
	* Get the height of the display in pixels.
	*/
	u8 GetHeight() const;

	/**
	* Gets the total amount of pixels in the display. (width * height)
	*/
	inline u16 GetSize() const { return (w_ * h_); }

private:
	u8 w_, h_;
	std::unique_ptr<u8[]> pix_;

	sf::Color displayColor_, backColor_;

	/**
	* Gets index from pixel co-ords.
	*/
	inline u16 GetPos(u8 x, u8 y) const { return (x + (y * w_)); }
};

