#include "terminal.h"
#include <iostream>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#endif

namespace datapainter {

// Store original terminal settings for restoration
#ifndef _WIN32
static struct termios orig_termios;
static bool raw_mode_enabled = false;
#endif

Terminal::Terminal() : rows_(24), cols_(80) {
    resize_buffer();
}

Terminal::~Terminal() = default;

void Terminal::set_dimensions(int rows, int cols) {
    rows_ = rows;
    cols_ = cols;
    resize_buffer();
}

bool Terminal::detect_size() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        cols_ = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        rows_ = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        resize_buffer();
        return true;
    }
    return false;
#else
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        rows_ = w.ws_row;
        cols_ = w.ws_col;
        resize_buffer();
        return true;
    }
    return false;
#endif
}

bool Terminal::is_size_adequate() const {
    // Need at least header + 3 rows for minimal display
    // And at least 40 columns for reasonable text
    return rows_ >= 5 && cols_ >= 40;
}

void Terminal::clear_buffer() {
    for (auto& row : buffer_) {
        std::fill(row.begin(), row.end(), ' ');
    }
}

void Terminal::write_char(int row, int col, char ch) {
    if (row >= 0 && row < rows_ && col >= 0 && col < cols_) {
        buffer_[row][col] = ch;
    }
}

char Terminal::read_char(int row, int col) const {
    if (row >= 0 && row < rows_ && col >= 0 && col < cols_) {
        return buffer_[row][col];
    }
    return ' ';
}

std::string Terminal::get_row(int row) const {
    if (row >= 0 && row < rows_) {
        return std::string(buffer_[row].begin(), buffer_[row].end());
    }
    return std::string(cols_, ' ');
}

void Terminal::render() {
    // Clear screen (ANSI escape code)
    std::cout << "\033[2J\033[H";

    // Output each row
    for (int row = 0; row < rows_; ++row) {
        std::cout << get_row(row);
        if (row < rows_ - 1) {
            std::cout << '\n';
        }
    }
    std::cout << std::flush;
}

void Terminal::resize_buffer() {
    // Save old buffer
    auto old_buffer = buffer_;
    int old_rows = old_buffer.size();
    int old_cols = old_rows > 0 ? old_buffer[0].size() : 0;

    // Create new buffer
    buffer_.clear();
    buffer_.resize(rows_);
    for (auto& row : buffer_) {
        row.resize(cols_, ' ');
    }

    // Copy old content that fits
    int copy_rows = std::min(old_rows, rows_);
    int copy_cols = std::min(old_cols, cols_);
    for (int r = 0; r < copy_rows; ++r) {
        for (int c = 0; c < copy_cols; ++c) {
            buffer_[r][c] = old_buffer[r][c];
        }
    }
}

bool Terminal::enter_raw_mode() {
#ifdef _WIN32
    // Windows: set console mode
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    if (!GetConsoleMode(hStdin, &mode)) {
        return false;
    }
    // Disable line input and echo
    SetConsoleMode(hStdin, mode & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT));
    return true;
#else
    // Unix: use termios
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
        return false;
    }

    struct termios raw = orig_termios;
    // Disable canonical mode and echo
    raw.c_lflag &= ~(ICANON | ECHO);
    // Set minimum characters and timeout for read
    raw.c_cc[VMIN] = 0;   // Non-blocking read
    raw.c_cc[VTIME] = 1;  // 100ms timeout

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        return false;
    }

    raw_mode_enabled = true;
    return true;
#endif
}

bool Terminal::exit_raw_mode() {
#ifdef _WIN32
    // Windows: restore original console mode
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    if (!GetConsoleMode(hStdin, &mode)) {
        return false;
    }
    SetConsoleMode(hStdin, mode | ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
    return true;
#else
    // Unix: restore original termios
    if (raw_mode_enabled) {
        if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) {
            return false;
        }
        raw_mode_enabled = false;
    }
    return true;
#endif
}

int Terminal::read_key() {
#ifdef _WIN32
    // Windows: use _kbhit() and _getch()
    if (_kbhit()) {
        return _getch();
    }
    return -1;  // No key available
#else
    // Unix: read from stdin
    unsigned char c;
    ssize_t nread = read(STDIN_FILENO, &c, 1);
    if (nread == 1) {
        return static_cast<int>(c);
    }
    return -1;  // No key available or error
#endif
}

} // namespace datapainter
