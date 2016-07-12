#include "Chip8Display.h"

#include <cassert>

#include <SFML\Graphics\RectangleShape.hpp>


Chip8Display::Chip8Display(const sf::Color& displayColor, const sf::Color& backColor, u8 w, u8 h) :
displayColor_(displayColor),
backColor_(backColor)
{
	Reset(w, h);
}


Chip8Display::~Chip8Display()
{
}


void Chip8Display::Reset(u8 w, u8 h)
{
	// Initialize and clear the display
	w_ = w;
	h_ = h;
	pix_ = std::unique_ptr<u8[]>(new u8[GetSize()]);
	Clear();
}


void Chip8Display::Clear()
{
	for (u16 i = 0; i < GetSize(); ++i)
	{
		pix_[i] = 0;
	}
}


void Chip8Display::Plot(u16 x, u16 y)
{
	// Toggle the pixel's on/off state by XORing against 1.
	// % operator handles pixels wrapping around to the other end of the screen if off screen.
	pix_[GetPos(x % w_, y % h_)] ^= 1;
}


u8 Chip8Display::GetPixelState(u16 x, u16 y)
{
	// % operator handles pixels wrapping around to the other end of the screen if off screen.
	return pix_[GetPos(x % w_, y % h_)];
}


void Chip8Display::Render(sf::RenderTarget& target)
{
	// Clear the screen to black.
	target.clear(backColor_);
	
	// Calculate individual pixel size from size of target view
	const auto pixWidth = target.getView().getSize().x / static_cast<float>(w_);
	const auto pixHeight = target.getView().getSize().y / static_cast<float>(h_);

	// Set the pixel color and size
	sf::RectangleShape pixRect;
	pixRect.setFillColor(displayColor_);
	pixRect.setSize(sf::Vector2f(pixWidth, pixHeight));

	// Draw active pixels.
	for (u8 x = 0; x < w_; ++x)
	{
		for (u8 y = 0; y < h_; ++y)
		{
			// If this pixel is on at this loc, draw it.
			if (pix_[GetPos(x, y)] != 0)
			{
				pixRect.setPosition(sf::Vector2f(x * pixWidth, y * pixHeight));
				target.draw(pixRect);
			}
		}
	}
}


void Chip8Display::SetDisplayColor(const sf::Color& color) 
{ 
	displayColor_ = color; 
}


sf::Color Chip8Display::GetDisplayColor() const 
{ 
	return displayColor_; 
}


void Chip8Display::SetBackgroundColor(const sf::Color& color) 
{ 
	backColor_ = color; 
}


sf::Color Chip8Display::GetBackgroundColor() const 
{ 
	return backColor_; 
}


u8 Chip8Display::GetWidth() const 
{ 
	return w_; 
}


u8 Chip8Display::GetHeight() const 
{ 
	return h_; 
}