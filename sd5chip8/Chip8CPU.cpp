#include "Chip8CPU.h"

#include "Chip8Beeper.h"
#include "Chip8Keyboard.h"
#include "Chip8Helper.h"

#include <cassert>
#include <iostream>
#include <sstream>


Chip8CPU::Chip8CPU(Chip8Memory& ram, Chip8Display& display, Chip8Beeper* beeper, bool isETI660) :
ram_(ram),
beeper_(beeper),
defaultSpritesAddr_(0),
display_(display),
isETI660_(isETI660),
rndDist_(0, 255)
{
	Reset();
}


Chip8CPU::~Chip8CPU()
{
}


void Chip8CPU::Reset()
{
	// Get the current time to use as seed for RNG and as
	// init for DT and ST decrement.
	const auto now = Chip8Helper::GetNowDuration();

	InitializeRegisters();
	lastStepTime_ = now;
	ResetTimerDecrement();
	assert(InitializeDefaultSprites());

	// Seed the CPU random number generator with current time.
	rnd_ = std::mt19937(static_cast<unsigned int>(now.count()));

	display_.Reset(CHIP8_DISPLAY_WIDTH, CHIP8_DISPLAY_HEIGHT);
	isInHiresMode_ = false;
	if (beeper_ != nullptr)
	{
		// Make sure the beeper isn't already beeping.
		beeper_->SetBeeping(false);
	}

	isWaitingForInput_ = false;
	lastOp_ = 0;
}


void Chip8CPU::InitializeRegisters()
{
	// Init PC to program start for chosen CPU type.
	reg_.PC = (isETI660_ ? CHIP8_PROGRAM_ETI660_START : CHIP8_PROGRAM_START);

	// Zero out the registers and stack.
	reg_.I = reg_.SP = reg_.DT = reg_.ST = 0;
	reg_.V[16] = {};
	reg_.stack[16] = {};
}


bool Chip8CPU::InitializeDefaultSprites(u16 writeAddr)
{
	defaultSpritesAddr_ = writeAddr;

	// Store default hexadecimal font sprite data into our reserved memory space.
	static const u8 fontSpriteData[80] = {
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	};

	// Write the default font sprite data into reserved memory, size: 80 (5 * 16).
	for (u16 i = 0; i < 80; ++i)
	{
		if (!ram_.WriteValue(i, fontSpriteData[i]))
		{
			// Failed to place default sprites into memory.
			return false;
		}
	}

	return true;
}


bool Chip8CPU::ExecuteOpSYS(u16 op)
{
	// This opcode is unused on modern interpreters as this opcode was used to call
	// old system functions on Chip-8 computers (such as RCA emulation mode).
	std::cout << "Ignoring SYS instruction: 0x" << std::hex << op << " (PC: 0x" << std::hex << reg_.PC << ")" << std::endl;
	SetPCNext();
	return true;
}


bool Chip8CPU::ExecuteOpCLS()
{
	display_.Clear();
	SetPCNext();
	return true;
}


bool Chip8CPU::ExecuteOpRET()
{
	if (reg_.SP > 16)
	{
		// Invalid SP - cannot decrement SP for RET because < 0 underflow or bad SP.
		std::cerr << "Invalid SP for RET instruction! (PC: 0x" << std::hex << reg_.PC << ", SP: 0x" << std::hex << +reg_.SP << ")" << std::endl;
		return false;
	}

	// Set PC to the instruction after the CALL we're returning from or we'll get an infinite loop.
	reg_.PC = reg_.stack[reg_.SP--] + 2;
	return true;
}


bool Chip8CPU::ExecuteOpJPAddr(u16 op)
{
	// Check if this is a Hires program - these usually start at 0x200 and immediately JP to 0x260.
	if (reg_.PC == CHIP8_PROGRAM_START && (op & 0x0FFF) == 0x260)
	{
		std::cout << "Program is initializing Hi-res mode." << std::endl;

		isInHiresMode_ = true;
		display_.Reset(CHIP8_HIRES_DISPLAY_WIDTH, CHIP8_HIRES_DISPLAY_HEIGHT);

		reg_.PC = CHIP8_PROGRAM_HIRES_START; // Jump to hires start instead.
		return true;
	}

	reg_.PC = (op & 0x0FFF);
	return true;
}


bool Chip8CPU::ExecuteOpCALL(u16 op)
{
	if (reg_.SP > 15)
	{
		// Invalid SP - cannot increment SP for CALL because no room on stack.
		std::cerr << "Invalid SP for CALL 0x" << std::hex << (op & 0x0FFF) << " instruction! (PC: 0x" << std::hex << reg_.PC << ", SP: 0x" << std::hex << +reg_.SP << ")" << std::endl;
		return false;
	}

	reg_.stack[++reg_.SP] = reg_.PC;
	reg_.PC = (op & 0x0FFF);
	return true;
}


bool Chip8CPU::ExecuteOpSEVxByte(u16 op)
{
	// If equal, skip next instruction.
	((reg_.V[GetXArg(op)] == (op & 0x00FF)) ? SetPCSkip() : SetPCNext());
	return true;
}


bool Chip8CPU::ExecuteOpSNEVxByte(u16 op)
{
	// If NOT equal, skip next instruction.
	((reg_.V[GetXArg(op)] != (op & 0x00FF)) ? SetPCSkip() : SetPCNext());
	return true;
}


bool Chip8CPU::ExecuteOpSEVxVy(u16 op)
{
	// If equal, skip next instruction.
	((reg_.V[GetXArg(op)] == reg_.V[GetYArg(op)]) ? SetPCSkip() : SetPCNext());
	return true;
}


bool Chip8CPU::ExecuteOpLDVxByte(u16 op)
{
	// Sets Vx to a value.
	reg_.V[GetXArg(op)] = (op & 0x00FF);
	SetPCNext();
	return true;
}


bool Chip8CPU::ExecuteOpADDVxByte(u16 op)
{
	// Sets Vx to Vx + val.
	reg_.V[GetXArg(op)] += (op & 0x00FF);
	SetPCNext();
	return true;
}


bool Chip8CPU::ExecuteOpLDVxVy(u16 op)
{
	// Sets Vx to Vy.
	reg_.V[GetXArg(op)] = reg_.V[GetYArg(op)];
	SetPCNext();
	return true;
}


bool Chip8CPU::ExecuteOpOR(u16 op)
{
	// bitwise OR Vx and Vy and store in Vx.
	reg_.V[GetXArg(op)] |= reg_.V[GetYArg(op)];
	SetPCNext();
	return true;
}


bool Chip8CPU::ExecuteOpAND(u16 op)
{
	// bitwise AND Vx and Vy and store in Vx.
	reg_.V[GetXArg(op)] &= reg_.V[GetYArg(op)];
	SetPCNext();
	return true;
}


bool Chip8CPU::ExecuteOpXOR(u16 op)
{
	// bitwise XOR Vx and Vy and store in Vx.
	reg_.V[GetXArg(op)] ^= reg_.V[GetYArg(op)];
	SetPCNext();
	return true;
}


bool Chip8CPU::ExecuteOpADDVxVy(u16 op)
{
	if ((255 - reg_.V[GetXArg(op)]) < reg_.V[GetYArg(op)])
	{
		// Overflow! Set VF to 1 to signal this.
		reg_.V[0xF] = 1;
	}
	else
	{
		// No overflow. Set VF to 0.
		reg_.V[0xF] = 0;
	}

	reg_.V[GetXArg(op)] += reg_.V[GetYArg(op)];
	SetPCNext();
	return true;
}


bool Chip8CPU::ExecuteOpSUB(u16 op)
{
	if (reg_.V[GetXArg(op)] > reg_.V[GetYArg(op)])
	{
		// Set VF to 1 to signal Vx > Vy.
		reg_.V[0xF] = 1;
	}
	else
	{
		// Set VF to 0 to signal Vx < Vy - underflow or 0 value warning!
		reg_.V[0xF] = 0;
	}

	reg_.V[GetXArg(op)] -= reg_.V[GetYArg(op)];
	SetPCNext();
	return true;
}


bool Chip8CPU::ExecuteOpSHR(u16 op)
{
	// If value of least-significant bit at Vx is 1, set VF to 1, otherwise 0.
	// Then perform integer DIV by 2.
	reg_.V[0xF] = (reg_.V[GetXArg(op)] & 1);
	reg_.V[GetXArg(op)] /= 2;
	SetPCNext();
	return true;
}


bool Chip8CPU::ExecuteOpSUBN(u16 op)
{
	if (reg_.V[GetXArg(op)] < reg_.V[GetYArg(op)])
	{
		// Set VF to 1 to signal Vy > Vx.
		reg_.V[0xF] = 1;
	}
	else
	{
		// Set VF to 0 to signal Vy < Vx - underflow or 0 value warning!
		reg_.V[0xF] = 0;
	}

	reg_.V[GetXArg(op)] = reg_.V[GetYArg(op)] - reg_.V[GetXArg(op)];
	SetPCNext();
	return true;
}


bool Chip8CPU::ExecuteOpSHL(u16 op)
{
	// If value of most-significant bit at Vx is 1, set VF to 1, otherwise 0.
	// Then perform multiplication by 2.
	reg_.V[0xF] = (reg_.V[GetXArg(op)] & 128) >> 7;
	reg_.V[GetXArg(op)] *= 2;
	SetPCNext();
	return true;
}


bool Chip8CPU::ExecuteOpSNEVxVy(u16 op)
{
	// If NOT equal, skip next instruction.
	((reg_.V[GetXArg(op)] != reg_.V[GetYArg(op)]) ? SetPCSkip() : SetPCNext());
	return true;
}


bool Chip8CPU::ExecuteOpLDIAddr(u16 op)
{
	// Sets I to addr.
	reg_.I = (op & 0x0FFF);
	SetPCNext();
	return true;
}


bool Chip8CPU::ExecuteOpJPV0Addr(u16 op)
{
	reg_.PC = (op & 0x0FFF) + reg_.V[0];
	return true;
}


bool Chip8CPU::ExecuteOpRND(u16 op)
{
	reg_.V[GetXArg(op)] = (static_cast<u8>(rndDist_(rnd_)) & (op & 0x00FF));
	SetPCNext();
	return true;
}


bool Chip8CPU::ExecuteOpDRW(u16 op)
{
	// NOTE: Each pixel of a sprite is stored as one bit, not a byte.
	// This function correctly handles this.

	const auto Vx = reg_.V[GetXArg(op)];
	const auto Vy = reg_.V[GetYArg(op)];

	// Assume no pixels have been toggled off AKA no collision...
	reg_.V[0xF] = 0;

	const u8 spriteLines = (op & 0x000F);	
	for (u8 y = 0; y < spriteLines; ++y)
	{
		u8 pixLine;
		if (!ram_.ReadValue(reg_.I + y, &pixLine))
		{
			// Failure reading sprite from memory
			std::cerr << "Could not read sprite for DRW V[0x" << std::hex << +GetXArg(op) << "], V[0x" << std::hex << +GetYArg(op) << "], " << +spriteLines
				<< " instruction! (PC: 0x" << std::hex << reg_.PC << ", I: 0x" << std::hex << reg_.I << ", Vx: " << +Vx << ", Vy: " << +Vy << ")" << std::endl;
			return false;
		}

		// Every sprite is 8 px in width.
		for (u8 x = 0; x < 8; ++x)
		{
			// Check that this pixel in the sprite isn't in an "off" state.
			if ((pixLine & (128 >> x)) == 0)
			{
				continue;
			}

			if (display_.GetPixelState(Vx + x, Vy + y) != 0)
			{
				// Pixel is already "on" - collision.
				reg_.V[0xF] = 1;
			}

			// Draw the pixel.
			display_.Plot(Vx + x, Vy + y);
		}
	}

	SetPCNext();
	return true;
}


bool Chip8CPU::ExecuteOpSKP(u16 op)
{
	// Skip next instruction if key with code at Vx is down.
	((Chip8Keyboard::isKeyDown(reg_.V[GetXArg(op)])) ? SetPCSkip() : SetPCNext());
	return true;
}


bool Chip8CPU::ExecuteOpSKNP(u16 op)
{
	// Skip next instruction if key with code at Vx is NOT down (key is up).
	((!Chip8Keyboard::isKeyDown(reg_.V[GetXArg(op)])) ? SetPCSkip() : SetPCNext());
	return true;
}


bool Chip8CPU::ExecuteOpLDVxDT(u16 op)
{
	reg_.V[GetXArg(op)] = reg_.DT;
	SetPCNext();
	return true;
}


bool Chip8CPU::ExecuteOpLDVxKey(u16 op)
{
	u8 key;
	if (!Chip8Keyboard::getCurrentPressedKey(&key))
	{
		// Wait until key press.
		isWaitingForInput_ = true;
		return true;
	}

	reg_.V[GetXArg(op)] = key;
	isWaitingForInput_ = false;
	SetPCNext();
	return true;
}


bool Chip8CPU::ExecuteOpLDDTVx(u16 op)
{
	reg_.DT = reg_.V[GetXArg(op)];
	SetPCNext();
	return true;
}


bool Chip8CPU::ExecuteOpLDSTVx(u16 op)
{
	reg_.ST = reg_.V[GetXArg(op)];
	SetPCNext();
	return true;
}


bool Chip8CPU::ExecuteOpADDIVx(u16 op)
{
	reg_.I += reg_.V[GetXArg(op)];
	SetPCNext();
	return true;
}


bool Chip8CPU::ExecuteOpLDFVx(u16 op)
{
	reg_.I = defaultSpritesAddr_ + (reg_.V[GetXArg(op)] * 5);
	SetPCNext();
	return true;
}


bool Chip8CPU::ExecuteOpLDBVx(u16 op)
{
	const auto Vx = reg_.V[GetXArg(op)];
	if (!(ram_.WriteValue(reg_.I, Vx / 100) &&
		ram_.WriteValue(reg_.I + 1, (Vx % 100) / 10) &&
		ram_.WriteValue(reg_.I + 2, (Vx % 100) % 10)))
	{
		// Failed to write to memory.
		std::cerr << "Could not write BCD for LD B, V[0x" << std::hex << +GetXArg(op) << "] instruction! (PC: 0x"
			<< std::hex << reg_.PC << ", I: 0x" << std::hex << reg_.I << ", Vx: " << +Vx << ")" << std::endl;
		return false;
	}

	SetPCNext();
	return true;
}


bool Chip8CPU::ExecuteOpLDIaddrVx(u16 op)
{
	// Loop through V0 through Vx
	for (u8 i = 0; i <= GetXArg(op); ++i)
	{
		// Try to write to I + i.
		if (!ram_.WriteValue(reg_.I + i, reg_.V[i]))
		{
			// Failure writing to memory.
			std::cerr << "Could not write values of V for LD [I], V[0x" << std::hex << +GetXArg(op) << "] instruction! (PC: 0x"
				<< std::hex << reg_.PC << ", I: 0x" << std::hex << reg_.I << ")" << std::endl;
			return false;
		}
	}

	SetPCNext();
	return true;
}


bool Chip8CPU::ExecuteOpLDVxIaddr(u16 op)
{
	// Loop through V0 through Vx
	for (u8 i = 0; i <= GetXArg(op); ++i)
	{
		// Try to read to Vx.
		if (!ram_.ReadValue(reg_.I + i, &reg_.V[i]))
		{
			// Failure reading from memory.
			std::cerr << "Could not read memory to V for LD V[0x" << std::hex << +GetXArg(op) << "], [I] instruction! (PC: 0x"
				<< std::hex << reg_.PC << ", I: 0x" << std::hex << reg_.I << ")" << std::endl;
			return false;
		}
	}

	SetPCNext();
	return true;
}


bool Chip8CPU::ExecuteOpcode(u16 op)
{
	//std::cout << "Op 0x" << std::hex << op << " (PC: 0x" << std::hex << reg_.PC << ", SP: 0x" << std::hex << +reg_.SP << ", I: 0x" << std::hex << reg_.I << ")" << std::endl;

	switch (op & 0xF000)
	{
	case 0x0000:
		switch (op & 0x00FF)
		{
		case 0x00E0:
			return ExecuteOpCLS();
		case 0x00EE:
			return ExecuteOpRET();
		default:
			return ExecuteOpSYS(op);
		}

	case 0x1000:
		return ExecuteOpJPAddr(op);
	case 0x2000:
		return ExecuteOpCALL(op);
	case 0x3000:
		return ExecuteOpSEVxByte(op);
	case 0x4000:
		return ExecuteOpSNEVxByte(op);
	case 0x5000:
		return ExecuteOpSEVxVy(op);
	case 0x6000:
		return ExecuteOpLDVxByte(op);
	case 0x7000:
		return ExecuteOpADDVxByte(op);

	case 0x8000:
		switch (op & 0x000F)
		{
		case 0x0000:
			return ExecuteOpLDVxVy(op);
		case 0x0001:
			return ExecuteOpOR(op);
		case 0x0002:
			return ExecuteOpAND(op);
		case 0x0003:
			return ExecuteOpXOR(op);
		case 0x0004:
			return ExecuteOpADDVxVy(op);
		case 0x0005:
			return ExecuteOpSUB(op);
		case 0x0006:
			return ExecuteOpSHR(op);
		case 0x0007:
			return ExecuteOpSUBN(op);
		case 0x000E:
			return ExecuteOpSHL(op);
		default:
			std::cerr << "Unknown opcode in 0x8000 group: 0x" << std::hex << op << "! (PC: 0x" << std::hex << reg_.PC << ")" << std::endl;
			return false;
		}

	case 0x9000:
		return ExecuteOpSNEVxVy(op);
	case 0xA000:
		return ExecuteOpLDIAddr(op);
	case 0xB000:
		return ExecuteOpJPV0Addr(op);
	case 0xC000:
		return ExecuteOpRND(op);
	case 0xD000:
		return ExecuteOpDRW(op);

	case 0xE000:
		switch (op & 0x00FF)
		{
		case 0x009E:
			return ExecuteOpSKP(op);
		case 0x00A1:
			return ExecuteOpSKNP(op);
		default:
			std::cerr << "Unknown opcode in 0xE000 group: 0x" << std::hex << op << "! (PC: 0x" << std::hex << reg_.PC << ")" << std::endl;
			return false;
		}

	case 0xF000:
		switch (op & 0x00FF)
		{
		case 0x0007:
			return ExecuteOpLDVxDT(op);
		case 0x000A:
			return ExecuteOpLDVxKey(op);
		case 0x0015:
			return ExecuteOpLDDTVx(op);
		case 0x0018:
			return ExecuteOpLDSTVx(op);
		case 0x001E:
			return ExecuteOpADDIVx(op);
		case 0x0029:
			return ExecuteOpLDFVx(op);
		case 0x0033:
			return ExecuteOpLDBVx(op);
		case 0x0055:
			return ExecuteOpLDIaddrVx(op);
		case 0x0065:
			return ExecuteOpLDVxIaddr(op);
		default:
			std::cerr << "Unknown opcode in 0xF000 group: 0x" << std::hex << op << "! (PC: 0x" << std::hex << reg_.PC << ")" << std::endl;
			return false;
		}

	default:
		std::cerr << "Unknown opcode: 0x" << std::hex << op << "! (PC: 0x" << std::hex << reg_.PC << ")" << std::endl;
		return false;
	}
}


bool Chip8CPU::Step()
{
	// Get the next opcode
	u16 op;
	const auto success = FetchOpcode(&op);
	if (!success)
	{
		return false;
	}

	lastOp_ = op;

	// Execute opcode
	if (!ExecuteOpcode(op))
	{
		return false;
	}

	// Only update DT and ST if not waiting for input.
	if (!isWaitingForInput_)
	{
		// Update DT and ST if needed.
		UpdateTimers();
	}

	return true;
}


bool Chip8CPU::RunFrame()
{
	for (int i = 0; i < CHIP8_CPU_STEPS_PER_FRAME; ++i)
	{
		if (!Step())
		{
			return false;
		}
	}

	return true;
}


void Chip8CPU::UpdateTimers()
{
	// Get current time.
	const auto now = Chip8Helper::GetNowDuration();

	// If it's time for a timer update...
	if (nextTimerDecrementCounter_.count() <= 0)
	{
		// Update delay timer if it's active
		if (reg_.DT > 0)
		{
			--reg_.DT;
		}

		// Update the sound timer if it's active and play a continous noise
		if (reg_.ST > 0)
		{
			--reg_.ST;
		}

		// Beep if ST > 0.
		if (beeper_ != nullptr)
		{
			beeper_->SetBeeping((reg_.ST > 0));
		}

		ResetTimerDecrement();
	}

	nextTimerDecrementCounter_ -= now - lastStepTime_;
	lastStepTime_ = now;
}


void Chip8CPU::ResetTimerDecrement()
{
	nextTimerDecrementCounter_ = std::chrono::microseconds(CHIP8_CPU_TIMER_DECREMENT_DELAY_MICROSECONDS);
}


bool Chip8CPU::FetchOpcode(u16* outOp) const
{
	u8 opBytes[2];
	const auto success = (ram_.ReadValue(reg_.PC, &opBytes[0]) && ram_.ReadValue(reg_.PC + 1, &opBytes[1]));
	if (!success)
	{
		return false;
	}

	if (outOp != nullptr)
	{
		*outOp = (opBytes[0] << 8) | opBytes[1];
	}
	return true;
}


void Chip8CPU::RenderCPUDebug(sf::RenderTarget& target, const sf::Font& font) const
{
	sf::Text debugText;
	debugText.setPosition(sf::Vector2f(0.0f, 0.0f));
	debugText.setCharacterSize(14);
	debugText.setFont(font);
	debugText.setColor(sf::Color(255, 0, 0));

	std::ostringstream oss;
	oss << "Op: 0x" << std::hex << lastOp_ << ", PC: 0x" << std::hex << reg_.PC << (isWaitingForInput_ ? " - WAITING FOR INPUT" : "") << std::endl
		<< "SP: 0x" << std::hex << +reg_.SP << ", I: 0x" << std::hex << reg_.I << std::endl
		<< "DT: 0x" << std::hex << +reg_.DT << ", ST: 0x" << std::hex << +reg_.ST << std::endl
		<< "V: ";
	for (u8 i = 0; i < 16; ++i)
	{
		oss << "0x" << std::hex << +reg_.V[i];
		if (i < 15)
		{
			oss << ", ";
		}
	}

	debugText.setString(oss.str());
	target.draw(debugText);
}


bool Chip8CPU::IsETI660Mode() const
{
	return isETI660_;
}


bool Chip8CPU::IsHiresMode() const
{
	return isInHiresMode_;
}


bool Chip8CPU::IsWaitingForInput() const
{
	return isWaitingForInput_;
}


u16 Chip8CPU::GetLastOpcode() const
{
	return lastOp_;
}