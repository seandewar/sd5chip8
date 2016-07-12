#pragma once

#include <chrono>

namespace Chip8Helper
{
	/**
	* Returns the current time since epoch.
	*/
	inline std::chrono::high_resolution_clock::duration GetNowDuration() { return std::chrono::high_resolution_clock::now().time_since_epoch(); }
};