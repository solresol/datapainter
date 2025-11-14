#include "terminal.h"
#include <iostream>
#include <algorithm>

#ifdef _WIN32
#define NOMINMAX  // Prevent windows.h from defining min/max macros
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

Terminal::Terminal() : rows_(24), cols_(80), actual_rows_(24), actual_cols_(80) {
    resize_buffer();
}

Terminal::~Terminal() {
    // Clean up ncurses if we initialized it
#ifndef _WIN32
    if (ncurses_initialized) {
        endwin();
        ncurses_initialized = false;
    }
#endif
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
        actual_cols_ = cols_;
        actual_rows_ = rows_;
        resize_buffer();
        return true;
    }
    return false;
#else
    // If ncurses is initialized, we need to properly handle resize
    if (ncurses_initialized) {
        // Get actual terminal size from the system
        struct winsize w;
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
            // Only call resizeterm() if the size actually changed
            // This prevents infinite resize loops
            if (w.ws_row != rows_ || w.ws_col != cols_) {
                // Tell ncurses about the new size using resizeterm()
                // This updates ncurses's internal data structures
                resizeterm(w.ws_row, w.ws_col);
            }

            // Now get the size from ncurses
            getmaxyx(stdscr, rows_, cols_);
            actual_rows_ = rows_;
            actual_cols_ = cols_;
            resize_buffer();
            return true;
        }
        return false;
    }

    // Otherwise use ioctl (non-ncurses mode)
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        rows_ = w.ws_row;
        cols_ = w.ws_col;
        actual_rows_ = rows_;
        actual_cols_ = cols_;
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

bool Terminal::validate_override_dimensions(int rows, int cols) const {
    // Override dimensions must not exceed actual terminal dimensions
    return rows <= actual_rows_ && cols <= actual_cols_;
}

void Terminal::clear_buffer() {
    for (auto& row : buffer_) {
        std::fill(row.begin(), row.end(), ' ');
    }
    for (auto& row : acs_buffer_) {
        std::fill(row.begin(), row.end(), AcsChar::NONE);
    }
}

void Terminal::write_char(int row, int col, char ch) {
    if (row >= 0 && row < rows_ && col >= 0 && col < cols_) {
        // Block wide characters (emoji, multi-byte UTF-8 sequences)
        // Only allow printable ASCII (32-126) and common control chars like tab/newline
        // Characters with high bit set (>127) are part of multi-byte UTF-8
        unsigned char uch = static_cast<unsigned char>(ch);
        if (uch > 127) {
            // Replace non-ASCII characters with '?'
            buffer_[row][col] = '?';
        } else {
            buffer_[row][col] = ch;
        }
        acs_buffer_[row][col] = AcsChar::NONE;  // Clear any ACS marker
    }
}

void Terminal::write_acs(int row, int col, Terminal::AcsChar acs_type) {
    if (row >= 0 && row < rows_ && col >= 0 && col < cols_) {
        acs_buffer_[row][col] = acs_type;
        // Store ASCII fallback in buffer for read_char() and tests
        switch (acs_type) {
            case AcsChar::ULCORNER:
            case AcsChar::URCORNER:
            case AcsChar::LLCORNER:
            case AcsChar::LRCORNER:
                buffer_[row][col] = '+';
                break;
            case AcsChar::HLINE:
                buffer_[row][col] = '-';
                break;
            case AcsChar::VLINE:
                buffer_[row][col] = '|';
                break;
            case AcsChar::NONE:
                break;
        }
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
            for (int col = 0; col < cols_ && col < COLS; ++col) {
                // Check if this position has an ACS character
                if (acs_buffer_[row][col] != AcsChar::NONE) {
                    chtype acs_ch;
                    switch (acs_buffer_[row][col]) {
                        case AcsChar::ULCORNER: acs_ch = ACS_ULCORNER; break;
                        case AcsChar::URCORNER: acs_ch = ACS_URCORNER; break;
                        case AcsChar::LLCORNER: acs_ch = ACS_LLCORNER; break;
                        case AcsChar::LRCORNER: acs_ch = ACS_LRCORNER; break;
                        case AcsChar::HLINE:    acs_ch = ACS_HLINE;    break;
                        case AcsChar::VLINE:    acs_ch = ACS_VLINE;    break;
                        default:                acs_ch = buffer_[row][col]; break;
                    }
                    mvaddch(row, col, acs_ch);
                } else {
                    // Regular character
                    mvaddch(row, col, buffer_[row][col]);
                }
            }
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
            for (int col = 0; col < cols_ && col < COLS; ++col) {
                bool is_cursor = (row == cursor_row && col == cursor_col);

                // Get the character to display
                chtype ch;
                if (acs_buffer_[row][col] != AcsChar::NONE) {
                    // Use ACS character
                    switch (acs_buffer_[row][col]) {
                        case AcsChar::ULCORNER: ch = ACS_ULCORNER; break;
                        case AcsChar::URCORNER: ch = ACS_URCORNER; break;
                        case AcsChar::LLCORNER: ch = ACS_LLCORNER; break;
                        case AcsChar::LRCORNER: ch = ACS_LRCORNER; break;
                        case AcsChar::HLINE:    ch = ACS_HLINE;    break;
                        case AcsChar::VLINE:    ch = ACS_VLINE;    break;
                        default:                ch = buffer_[row][col]; break;
                    }
                } else {
                    ch = buffer_[row][col];
                }

                // Render with or without cursor highlighting
                if (is_cursor) {
                    attron(A_REVERSE);
                    mvaddch(row, col, ch);
                    attroff(A_REVERSE);
                } else {
                    mvaddch(row, col, ch);
                }
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
    // Save old buffers
    auto old_buffer = buffer_;
    auto old_acs_buffer = acs_buffer_;
    int old_rows = static_cast<int>(old_buffer.size());
    int old_cols = old_rows > 0 ? static_cast<int>(old_buffer[0].size()) : 0;

    // Create new char buffer
    buffer_.clear();
    buffer_.resize(rows_);
    for (auto& row : buffer_) {
        row.resize(cols_, ' ');
    }

    // Create new ACS buffer
    acs_buffer_.clear();
    acs_buffer_.resize(rows_);
    for (auto& row : acs_buffer_) {
        row.resize(cols_, AcsChar::NONE);
    }

    // Copy old content that fits
    int copy_rows = std::min(old_rows, rows_);
    int copy_cols = std::min(old_cols, cols_);
    for (int r = 0; r < copy_rows; ++r) {
        for (int c = 0; c < copy_cols; ++c) {
            buffer_[r][c] = old_buffer[r][c];
            if (!old_acs_buffer.empty()) {
                acs_buffer_[r][c] = old_acs_buffer[r][c];
            }
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
        // Unset LINES and COLUMNS environment variables if they exist
        // These can interfere with ncurses's ability to detect terminal resizes
        unsetenv("LINES");
        unsetenv("COLUMNS");

        initscr();              // Initialize ncurses
        raw();                  // Disable line buffering
        noecho();               // Don't echo typed characters
        keypad(stdscr, TRUE);   // Enable function keys, arrow keys, etc.
        timeout(50);            // 50ms timeout for getch() (non-blocking with timeout)
        set_escdelay(25);       // Make ESC detection snappy for UI tests
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
        int ch;
        // Loop until we get an actual key (not timeout)
        // This makes read_key() truly blocking, which is correct for terminal input
        // Only FileInputSource should return -1 at EOF
        do {
            ch = getch();
        } while (ch == ERR);

        // ncurses translates arrow keys and special events to codes
        // Map them to our public key codes
        switch (ch) {
            case KEY_UP:    return KEY_UP_ARROW;
            case KEY_DOWN:  return KEY_DOWN_ARROW;
            case KEY_LEFT:  return KEY_LEFT_ARROW;
            case KEY_RIGHT: return KEY_RIGHT_ARROW;
#ifdef KEY_RESIZE
            case KEY_RESIZE: return 1004;  // Terminal::KEY_RESIZE (avoid macro expansion issue)
#endif
#ifdef KEY_DC
            case KEY_DC:    return 127;  // Delete key (forward delete) -> map to DEL
#endif
#ifdef KEY_BACKSPACE
            case KEY_BACKSPACE: return 127;  // Backspace key -> map to DEL
#endif
            default:        return ch;
        }
    }
    return -1;
#endif
}

} // namespace datapainter
