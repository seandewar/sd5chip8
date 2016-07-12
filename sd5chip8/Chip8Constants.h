#pragma once

#define CHIP8_EMULATOR_DEFAULT_FONT_FILENAME "consola.ttf"

#define CHIP8_WINDOW_WIDTH 800
#define CHIP8_WINDOW_HEIGHT 600

#define CHIP8_DISPLAY_WIDTH 64
#define CHIP8_DISPLAY_HEIGHT 32

#define CHIP8_HIRES_DISPLAY_WIDTH 64
#define CHIP8_HIRES_DISPLAY_HEIGHT 64

#define CHIP8_BEEPER_DEFAULT_SAMPLES 44100
#define CHIP8_BEEPER_DEFAULT_SAMPLE_RATE 44100
#define CHIP8_BEEPER_DEFAULT_AMPLITUDE 35000

#define CHIP8_MEMORY_SIZE 4096
#define CHIP8_MEMORY_ETI660_SIZE 2048

#define CHIP8_CPU_STEPS_PER_FRAME 8
#define CHIP8_CPU_TIMER_DECREMENT_DELAY_MICROSECONDS 16667 // Rate of around 60 Hz

#define CHIP8_FRAME_SLEEP_MICROSECONDS 16667 // Rate of around 60 Hz

#define CHIP8_PROGRAM_START 0x200
#define CHIP8_PROGRAM_ETI660_START 0x600
#define CHIP8_PROGRAM_HIRES_START 0x2C0
#define CHIP8_PROGRAM_DEFAULT_SPRITES_START 0x000