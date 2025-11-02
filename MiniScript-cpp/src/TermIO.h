#pragma once
#include <string>

namespace TermIO {

// Enable or disable raw mode. On Windows, also enables VT input and (optionally) VT output.
// Returns true on success, false on failure (see lastError()).
bool EnableRaw(bool on, bool enableVTOut = true);

// Read up to maxBytes from stdin. If timeoutMs < 0, block; else wait up to timeoutMs.
// Returns number of bytes read; 0 means timeout; -1 means error (see lastError()).
int Read(char* buf, int maxBytes, int timeoutMs = -1);

// Get terminal rows/cols. Returns true on success.
bool GetSize(int& rows, int& cols);

// Optional: last OS error message (thread-local).
const std::string& lastError();

} // namespace TermIO
