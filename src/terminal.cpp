#include "terminal.h"
#include <iostream>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <ncurses.h>
#include <sys/ioctl.h>
#include <unistd.h>
#endif

namespace datapainter {

// Track whether ncurses is initialized
static bool ncurses_initialized = false;

Terminal::Terminal() : rows_(24), cols_(80) {
    resize_buffer();
}

Terminal::~Terminal() {
    // Clean up ncurses if we initialized it
    if (ncurses_initialized) {
        endwin();
        ncurses_initialized = false;
    }
}

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
    // If ncurses is initialized, use its size detection
    if (ncurses_initialized) {
        getmaxyx(stdscr, rows_, cols_);
        resize_buffer();
        return true;
    }

    // Otherwise use ioctl
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
#ifndef _WIN32
    if (ncurses_initialized) {
        // Use ncurses for rendering
        clear();
        for (int row = 0; row < rows_ && row < LINES; ++row) {
            std::string line = get_row(row);
            mvaddnstr(row, 0, line.c_str(), std::min(cols_, COLS));
        }
        refresh();
        return;
    }
#endif

    // Fallback: use ANSI escape codes
    std::cout << "\033[2J\033[H";
    for (int row = 0; row < rows_; ++row) {
        std::cout << get_row(row);
        if (row < rows_ - 1) {
            std::cout << '\n';
        }
    }
    std::cout << std::flush;
}

void Terminal::render_with_cursor(int cursor_row, int cursor_col) {
#ifndef _WIN32
    if (ncurses_initialized) {
        // Use ncurses for rendering with cursor
        clear();
        for (int row = 0; row < rows_ && row < LINES; ++row) {
            std::string line = get_row(row);

            if (row == cursor_row && cursor_col >= 0 && cursor_col < cols_) {
                // Render line up to cursor
                if (cursor_col > 0) {
                    mvaddnstr(row, 0, line.c_str(), cursor_col);
                }
                // Render cursor with reverse video
                attron(A_REVERSE);
                mvaddch(row, cursor_col, line[cursor_col]);
                attroff(A_REVERSE);
                // Render rest of line
                if (cursor_col + 1 < cols_) {
                    mvaddnstr(row, cursor_col + 1, line.c_str() + cursor_col + 1,
                             std::min(cols_ - cursor_col - 1, COLS - cursor_col - 1));
                }
            } else {
                mvaddnstr(row, 0, line.c_str(), std::min(cols_, COLS));
            }
        }
        refresh();
        return;
    }
#endif

    // Fallback: use ANSI escape codes
    std::cout << "\033[2J\033[H";
    for (int row = 0; row < rows_; ++row) {
        std::string line = get_row(row);
        if (row == cursor_row && cursor_col >= 0 && cursor_col < cols_) {
            std::cout << line.substr(0, cursor_col);
            std::cout << "\033[7m" << line[cursor_col] << "\033[27m";
            if (cursor_col + 1 < cols_) {
                std::cout << line.substr(cursor_col + 1);
            }
        } else {
            std::cout << line;
        }
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
    // Unix: use ncurses
    if (!ncurses_initialized) {
        initscr();              // Initialize ncurses
        raw();                  // Disable line buffering
        noecho();               // Don't echo typed characters
        keypad(stdscr, TRUE);   // Enable function keys, arrow keys, etc.
        timeout(50);            // 50ms timeout for getch() (non-blocking with timeout)
        curs_set(0);            // Hide the default cursor (we'll draw our own)

        ncurses_initialized = true;

        // Update dimensions from ncurses
        detect_size();
    }
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
    // Unix: cleanup ncurses
    if (ncurses_initialized) {
        endwin();
        ncurses_initialized = false;
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
    // Unix: use ncurses getch()
    if (ncurses_initialized) {
        int ch = getch();
        // getch() returns ERR if no key (due to timeout)
        if (ch == ERR) {
            return -1;
        }

        // ncurses translates arrow keys to special codes
        // Map them to our public key codes
        switch (ch) {
            case KEY_UP:    return KEY_UP_ARROW;
            case KEY_DOWN:  return KEY_DOWN_ARROW;
            case KEY_LEFT:  return KEY_LEFT_ARROW;
            case KEY_RIGHT: return KEY_RIGHT_ARROW;
            default:        return ch;
        }
    }
    return -1;
#endif
}

} // namespace datapainter
