// Manual test program to debug terminal resize behavior
// Compile and run this manually, then resize the terminal to see what happens

#include <iostream>
#include <cstdlib>
#include "../include/terminal.h"

int main() {
    std::cout << "Terminal Resize Debug Tool\n";
    std::cout << "==========================\n\n";

    // Check environment variables before we start
    const char* lines_env = std::getenv("LINES");
    const char* cols_env = std::getenv("COLUMNS");
    std::cout << "Initial LINES env: " << (lines_env ? lines_env : "not set") << "\n";
    std::cout << "Initial COLUMNS env: " << (cols_env ? cols_env : "not set") << "\n\n";

    datapainter::Terminal terminal;

    // Detect initial size
    if (terminal.detect_size()) {
        std::cout << "Initial terminal size: "
                  << terminal.rows() << "x" << terminal.cols() << "\n\n";
    }

    // Enter raw mode (which should unset LINES/COLUMNS)
    if (!terminal.enter_raw_mode()) {
        std::cerr << "Failed to enter raw mode\n";
        return 1;
    }

    // Check environment variables after entering raw mode
    lines_env = std::getenv("LINES");
    cols_env = std::getenv("COLUMNS");
    std::cout << "After enter_raw_mode() LINES env: " << (lines_env ? lines_env : "not set") << "\n";
    std::cout << "After enter_raw_mode() COLUMNS env: " << (cols_env ? cols_env : "not set") << "\n\n";

    // Detect size again
    if (terminal.detect_size()) {
        std::cout << "Size after enter_raw_mode(): "
                  << terminal.rows() << "x" << terminal.cols() << "\n\n";
    }

    std::cout << "Instructions:\n";
    std::cout << "1. Resize your terminal window now\n";
    std::cout << "2. Press any key after resizing (or 'q' to quit)\n";
    std::cout << "3. Watch for KEY_RESIZE events\n\n";

    bool running = true;
    int resize_count = 0;

    while (running) {
        int key = terminal.read_key();

        if (key >= 0) {
            if (key == datapainter::Terminal::KEY_RESIZE) {
                resize_count++;
                terminal.detect_size();
                std::cout << "\n*** RESIZE EVENT #" << resize_count << " detected! ***\n";
                std::cout << "New size: " << terminal.rows() << "x" << terminal.cols() << "\n\n";
            } else if (key == 'q' || key == 'Q') {
                std::cout << "\nQuitting...\n";
                running = false;
            } else if (key == 12) {  // Ctrl-L
                terminal.detect_size();
                std::cout << "\n*** Manual refresh (Ctrl-L) ***\n";
                std::cout << "Current size: " << terminal.rows() << "x" << terminal.cols() << "\n\n";
            } else {
                std::cout << "Key pressed: " << key;
                if (key >= 32 && key <= 126) {
                    std::cout << " ('" << (char)key << "')";
                }
                std::cout << "\n";
            }
        }
    }

    terminal.exit_raw_mode();

    std::cout << "\nTotal resize events detected: " << resize_count << "\n";

    return 0;
}
