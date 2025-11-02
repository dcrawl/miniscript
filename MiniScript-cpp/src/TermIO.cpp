#include "TermIO.h"
#include <string>
#include <atomic>

/*
Windows: Turning on ENABLE_VIRTUAL_TERMINAL_INPUT means arrow/function keys arrive as ANSI/VT 
sequences you can parse in MiniScript; turning on ENABLE_VIRTUAL_TERMINAL_PROCESSING lets your 
TUI use color/move/scroll sequences. Both behaviors are documented by Microsoft; VT sequences are 
processed when these modes are set. 
[https://learn.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences]

POSIX/macOS: cfmakeraw is the standard way to enter raw mode; VMIN/VTIME define blocking semantics; 
we use select() to implement timeouts cleanly before calling read(). TIOCGWINSZ gives you rows/cols. 
[https://www.man7.org/linux/man-pages/man3/termios.3.html]
*/
#if _WIN32
  #define NOMINMAX
  #include <windows.h>
#else
  #include <unistd.h>
  #include <termios.h>
  #include <sys/ioctl.h>
  #include <sys/select.h>
  #include <errno.h>
  #include <string.h>
#endif

namespace {
  thread_local std::string g_lastErr;
  inline void setErr(const std::string& s) { g_lastErr = s; }
}

namespace TermIO {

const std::string& lastError() { return g_lastErr; }

#if _WIN32

static std::atomic<bool> g_raw{false};
static DWORD g_inModeOrig = 0, g_outModeOrig = 0;
static HANDLE g_hIn  = INVALID_HANDLE_VALUE;
static HANDLE g_hOut = INVALID_HANDLE_VALUE;

static bool ensureHandles() {
    if (g_hIn == INVALID_HANDLE_VALUE)  g_hIn  = GetStdHandle(STD_INPUT_HANDLE);
    if (g_hOut == INVALID_HANDLE_VALUE) g_hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (g_hIn == INVALID_HANDLE_VALUE || g_hOut == INVALID_HANDLE_VALUE) {
        setErr("GetStdHandle failed");
        return false;
    }
    return true;
}

bool EnableRaw(bool on, bool enableVTOut) {
    if (!ensureHandles()) return false;

    if (on) {
        if (!g_raw.load()) {
            if (!GetConsoleMode(g_hIn, &g_inModeOrig))  { setErr("GetConsoleMode(in) failed");  return false; }
            if (!GetConsoleMode(g_hOut, &g_outModeOrig)) { setErr("GetConsoleMode(out) failed"); return false; }
        }
        DWORD inMode = g_inModeOrig;
        // Disable line/echo/processed input; enable VT input (so we receive escape sequences)
        inMode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT | ENABLE_QUICK_EDIT_MODE);
        inMode |=  (ENABLE_VIRTUAL_TERMINAL_INPUT);
        if (!SetConsoleMode(g_hIn, inMode)) { setErr("SetConsoleMode(in) failed"); return false; }

        if (enableVTOut) {
            DWORD outMode = g_outModeOrig;
            // Enable VT output processing; keep processed output
            outMode |= ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            // (Optionally: outMode |= DISABLE_NEWLINE_AUTO_RETURN; // if you prefer xterm-like EOL behavior)
            if (!SetConsoleMode(g_hOut, outMode)) { setErr("SetConsoleMode(out) failed"); return false; }
        }
        g_raw.store(true);
        return true;
    } else {
        if (g_raw.load()) {
            // Restore original modes
            if (!SetConsoleMode(g_hIn,  g_inModeOrig))  { setErr("Restore in mode failed");  return false; }
            if (!SetConsoleMode(g_hOut, g_outModeOrig)) { setErr("Restore out mode failed"); return false; }
            g_raw.store(false);
        }
        return true;
    }
}

int Read(char* buf, int maxBytes, int timeoutMs) {
    if (!ensureHandles()) return -1;
    if (maxBytes <= 0) return 0;

    DWORD rdy = WAIT_OBJECT_0;
    if (timeoutMs >= 0) {
        rdy = WaitForSingleObject(g_hIn, (DWORD)timeoutMs);
        if (rdy == WAIT_TIMEOUT) return 0;
        if (rdy != WAIT_OBJECT_0) { setErr("WaitForSingleObject failed"); return -1; }
    }
    DWORD read = 0;
    if (!ReadFile(g_hIn, buf, (DWORD)maxBytes, &read, NULL)) {
        setErr("ReadFile failed");
        return -1;
    }
    return (int)read;
}

bool GetSize(int& rows, int& cols) {
    if (!ensureHandles()) return false;
    CONSOLE_SCREEN_BUFFER_INFO info{};
    if (!GetConsoleScreenBufferInfo(g_hOut, &info)) { setErr("GetConsoleScreenBufferInfo failed"); return false; }
    rows = info.srWindow.Bottom - info.srWindow.Top + 1;
    cols = info.srWindow.Right  - info.srWindow.Left + 1;
    return true;
}

#else // POSIX (macOS, Linux)

static std::atomic<bool> g_raw{false};
static struct termios g_orig {};
static bool g_haveOrig = false;

bool EnableRaw(bool on, bool /*enableVTOut*/) {
    if (on) {
        if (!g_raw.load()) {
            if (tcgetattr(STDIN_FILENO, &g_orig) != 0) { setErr("tcgetattr failed: " + std::string(strerror(errno))); return false; }
            g_haveOrig = true;
            struct termios raw = g_orig;
            cfmakeraw(&raw);               // canonical off, echo off, signals off, etc.  [3](https://www.man7.org/linux/man-pages/man3/termios.3.html)[4](https://linux.die.net/man/3/cfmakeraw)
            raw.c_cc[VMIN]  = 1;           // block for at least 1 byte
            raw.c_cc[VTIME] = 0;           // no interbyte timeout
            if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) != 0) { setErr("tcsetattr failed: " + std::string(strerror(errno))); return false; }
        }
        g_raw.store(true);
        return true;
    } else {
        if (g_raw.load() && g_haveOrig) {
            if (tcsetattr(STDIN_FILENO, TCSANOW, &g_orig) != 0) { setErr("restore tcsetattr failed: " + std::string(strerror(errno))); return false; }
            g_raw.store(false);
        }
        return true;
    }
}

int Read(char* buf, int maxBytes, int timeoutMs) {
    if (maxBytes <= 0) return 0;

    if (timeoutMs >= 0) {
        fd_set rfds; FD_ZERO(&rfds); FD_SET(STDIN_FILENO, &rfds);
        struct timeval tv;
        tv.tv_sec  = timeoutMs / 1000;
        tv.tv_usec = (timeoutMs % 1000) * 1000;
        int rv = select(STDIN_FILENO+1, &rfds, nullptr, nullptr, &tv);
        if (rv == 0) return 0;            // timeout
        if (rv < 0) { setErr("select failed: " + std::string(strerror(errno))); return -1; }
    }
    ssize_t n = ::read(STDIN_FILENO, buf, (size_t)maxBytes);
    if (n < 0) { setErr("read failed: " + std::string(strerror(errno))); return -1; }
    return (int)n;
}

bool GetSize(int& rows, int& cols) {
    struct winsize ws{};
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) { setErr("ioctl TIOCGWINSZ failed: " + std::string(strerror(errno))); return false; }
    rows = ws.ws_row; cols = ws.ws_col;
    return true;
}

#endif

} // namespace TermIO
