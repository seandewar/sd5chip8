#pragma once

#include "Chip8Types.h"
#include "Chip8Memory.h"
#include "Chip8Display.h"

#include <random>
#include <chrono>

#include <SFML\Graphics\Font.hpp>
#include <SFML\Graphics\Text.hpp>

/**
* POD struct that contains the registers used by the Chip-8 CPU.
*/
struct Chip8CPURegisters
{
	/* Pseudo-registers */
	u16 PC;	// The Program Counter (PC)
	u8 SP;	// The Stack Pointer (SP)

	/* General purpose registers */
	u8 V[16]; // The 16 general purpose registers, V[0xF] is used by the CPU as flags for some instructions.

	/* Stack */
	u16 stack[16]; // Defines the stack (16, 16-bit values)

	/* Other registers */
	u16 I;	// The I register (generally used to store memory addresses)

	/* Sound registers */
	u8 DT;	// The delay timer register
	u8 ST;	// The sound timer register
};


class Chip8Beeper;

/**
* Contains the implementation of the Chip-8 CPU.
*/
class Chip8CPU
{
public:
	Chip8CPU(Chip8Memory& ram, Chip8Display& display, Chip8Beeper* beeper, bool isETI660 = false);
	~Chip8CPU();

	/**
	* Initializes the CPU.
	*/
	void Reset();

	/**
	* Executes one frame's worth of steps.
	*/
	bool RunFrame();

	/**
	* Executes the next program opcode in RAM.
	* Returns true if successful, false if there was an error.
	*/
	bool Step();

	/**
	* Returns whether or not the CPU is emulating the ETI 660 computer.
	*/
	bool IsETI660Mode() const;

	/**
	* Returns whether or not the CPU is in Hires mode (is running a Hires program).
	*/
	bool IsHiresMode() const;

	/**
	* Returns whether or not the CPU is currently halting execution because it is waiting for user input
	* because of the LD Vx, K instruction.
	*/
	bool IsWaitingForInput() const;

	/**
	* Renders CPU debug information onto a target.
	*/
	void RenderCPUDebug(sf::RenderTarget& target, const sf::Font& font) const;

	/**
	* Returns the last executed opcode.
	*/
	u16 GetLastOpcode() const;

private:
	Chip8CPURegisters reg_;
	Chip8Memory& ram_;
	Chip8Display& display_;
	Chip8Beeper* beeper_;
	u16 defaultSpritesAddr_;

	const bool isETI660_;
	bool isInHiresMode_;
	bool isWaitingForInput_;
	u16 lastOp_;

	std::mt19937 rnd_;
	std::uniform_int_distribution<short> rndDist_;
	std::chrono::high_resolution_clock::duration lastStepTime_;
	std::chrono::high_resolution_clock::duration nextTimerDecrementCounter_;

	/**
	* Initializes registers. Sets PC to value depending on if CPU is ETI 660 or not.
	*/
	void InitializeRegisters();

	/**
	* Writes the default sprites into program memory (size 0x050).
	*/
	bool InitializeDefaultSprites(u16 writeAddr = CHIP8_PROGRAM_DEFAULT_SPRITES_START);

	/**
	* Resets the timer decrement counter.
	*/
	void ResetTimerDecrement();

	/**
	* Updates the DT and ST timers.
	*/
	void UpdateTimers();

	/**
	* Fetches the next opcode in program memory.
	* Writes to outOp if it is not null.
	* Returns true if successful, false if not.
	*/
	bool FetchOpcode(u16* outOp) const;

	/**
	* Gets the x argument from an opcode.
	*/
	inline u8 GetXArg(u16 op) const { return ((op & 0x0F00) >> 8); }

	/**
	* Gets the y argument from an opcode.
	*/
	inline u8 GetYArg(u16 op) const { return ((op & 0x00F0) >> 4); }

	/**
	* Sets the PC to the next opcode by incrementing it by 2.
	*/
	inline void SetPCNext() { reg_.PC += 2; }

	/**
	* Sets the PC to skip the next opcode by incrementing it by 4.
	*/
	inline void SetPCSkip() { reg_.PC += 4; }

	/**
	* Executes the specified opcode.
	* Returns true on success, false on failure.
	*/
	bool ExecuteOpcode(u16 op);

	/**
	* Executes the SYS opcode - unused.
	*/
	bool ExecuteOpSYS(u16 op);

	/**
	* Executes the CLS opcode - clears the screen.
	*/
	bool ExecuteOpCLS();

	/**
	* Executes the RET opcode - returns from a subroutine.
	*/
	bool ExecuteOpRET();

	/**
	* Executes the JP addr opcode - jumps to an address in memory.
	*/
	bool ExecuteOpJPAddr(u16 op);

	/**
	* Executes the CALL opcode - calls a subroutine.
	*/
	bool ExecuteOpCALL(u16 op);

	/**
	* Executes the SE Vx, byte opcode - skips next instruction on true condition.
	*/
	bool ExecuteOpSEVxByte(u16 op);

	/**
	* Executes the SNE Vx, byte opcode - skips the next instruction on false condition.
	*/
	bool ExecuteOpSNEVxByte(u16 op);

	/**
	* Executes the SE Vx, Vy opcode - skips the next instruction on true condition.
	*/
	bool ExecuteOpSEVxVy(u16 op);

	/**
	* Executes the LD Vx, byte opcode - sets the value of Vx.
	*/
	bool ExecuteOpLDVxByte(u16 op);

	/**
	* Executes the ADD Vx, byte opcode - sets Vx to Vx + val.
	*/
	bool ExecuteOpADDVxByte(u16 op);

	/**
	* Executes the LD Vx, Vy opcode - sets the value of Vx to Vy.
	*/
	bool ExecuteOpLDVxVy(u16 op);

	/**
	* Executes the OR opcode - performs bitwise OR on Vx and Vy - stores result in Vx.
	*/
	bool ExecuteOpOR(u16 op);

	/**
	* Executes the AND opcode - performs a bitwise AND on Vx and Vy - stores result in Vx.
	*/
	bool ExecuteOpAND(u16 op);

	/**
	* Executes the XOR opcode - performs a bitwise XOR on Vx and Vy - stores result in Vx.
	*/
	bool ExecuteOpXOR(u16 op);

	/**
	* Executes the ADD Vx, Vy opcode - sets Vx to Vx + Vy. Sets VF to 1 on overflow.
	*/
	bool ExecuteOpADDVxVy(u16 op);

	/**
	* Executes the SUB opcode - sets Vx to Vx - Vy. Sets VF to 1 if Vx > Vy initially, otherwise 0. AKA VF is 0 if underflow or 0 result.
	*/
	bool ExecuteOpSUB(u16 op);

	/**
	* Executes the SHR opcode - sets VF to 1 if the least-significant bit of Vx is 1, otherwise 0. Then divides Vx by 2.
	*/
	bool ExecuteOpSHR(u16 op);

	/**
	* Executes the SUBN opcode - sets Vx to Vy - Vx. Sets VF to 1 if Vy > Vx initially, otherwise 0. AKA VF is 0 if underflow or 0 result.
	*/
	bool ExecuteOpSUBN(u16 op);

	/**
	* Executes the SHL opcode - sets VF to 1 if the most-significant bit of Vx is 1, otherwise 0. Then multiplies Vx by 2.
	*/
	bool ExecuteOpSHL(u16 op);

	/**
	* Executes the SNE Vx, Vy opcode - skips the next instruction on false condition.
	*/
	bool ExecuteOpSNEVxVy(u16 op);

	/**
	* Executes the LD I, addr opcode - sets the value of I to a value.
	*/
	bool ExecuteOpLDIAddr(u16 op);

	/**
	* Executes the JP V0, addr opcode - jumps to an address in memory + V0.
	*/
	bool ExecuteOpJPV0Addr(u16 op);

	/**
	* Executes the RND opcode - sets Vx to a random byte and then bitwise ANDs it with a value.
	*/
	bool ExecuteOpRND(u16 op);

	/**
	* Executes the DRW opcode - displays sprite starting at address I to (I + n) at co-ords (Vx, Vy).
	* VF is set to 1 if it causes any pixels that are already on to be toggled off, otherwise 0.
	*/
	bool ExecuteOpDRW(u16 op);

	/**
	* Executes the SKP opcode - skips the next opcode if key with value Vx is down.
	*/
	bool ExecuteOpSKP(u16 op);

	/**
	* Executes the SKNP opcode - skips the next opcode if the key with value Vx is up.
	*/
	bool ExecuteOpSKNP(u16 op);

	/**
	* Executes the LD Vx, DT opcode - sets Vx to DT.
	*/
	bool ExecuteOpLDVxDT(u16 op);

	/**
	* Executes the LD Vx, Key opcode - sets Vx to the value of a key that is pressed. Pauses execution until key is pressed.
	*/
	bool ExecuteOpLDVxKey(u16 op);

	/**
	* Executes the LD DT, Vx opcode - sets DT to Vx.
	*/
	bool ExecuteOpLDDTVx(u16 op);

	/**
	* Executes the LD ST, Vx opcode - sets ST to Vx.
	*/
	bool ExecuteOpLDSTVx(u16 op);

	/**
	* Executes the ADD I, Vx opcode - sets I to I + Vx.
	*/
	bool ExecuteOpADDIVx(u16 op);

	/**
	* Executes the LD F, Vx opcode - sets I to location of default sprite at index Vx.
	*/
	bool ExecuteOpLDFVx(u16 op);

	/**
	* Executes the LD B, Vx opcode - takes the decimal value of Vx, places the hundreds digit in I, tens in (I + 1) and ones in (I + 2).
	*/
	bool ExecuteOpLDBVx(u16 op);

	/**
	* Executes the LD [I], Vx opcode - stores registers V0 through Vx in memory at location I.
	*/
	bool ExecuteOpLDIaddrVx(u16 op);

	/**
	* Executes the LD Vx, [I] opcode - reads registers V0 through Vx from memory starting at location I.
	*/
	bool ExecuteOpLDVxIaddr(u16 op);
};

