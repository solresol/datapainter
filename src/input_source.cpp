#include "input_source.h"
#include "terminal.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace datapainter {

// TerminalInputSource implementation

TerminalInputSource::TerminalInputSource(Terminal& terminal)
    : terminal_(terminal) {}

int TerminalInputSource::read_key() {
    return terminal_.read_key();
}

bool TerminalInputSource::has_input() const {
    // Terminal always has potential input (blocks until key is pressed)
    return true;
}

// FileInputSource implementation

FileInputSource::FileInputSource(const std::string& filename)
    : current_index_(0) {
    if (!parse_file(filename)) {
        // error_ is set by parse_file
    }
}

int FileInputSource::read_key() {
    if (current_index_ >= keystrokes_.size()) {
        return -1;  // EOF
    }
    return keystrokes_[current_index_++];
}

bool FileInputSource::has_input() const {
    return current_index_ < keystrokes_.size();
}

bool FileInputSource::parse_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        error_ = "Could not open file: " + filename;
        return false;
    }

    std::string line;
    int line_number = 0;
    while (std::getline(file, line)) {
        line_number++;

        // Trim leading/trailing whitespace
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }

        // Parse the keystroke
        auto keystroke = parse_keystroke(line);
        if (!keystroke.has_value()) {
            std::ostringstream oss;
            oss << "Invalid keystroke on line " << line_number << ": " << line;
            error_ = oss.str();
            return false;
        }

        keystrokes_.push_back(keystroke.value());
    }

    if (keystrokes_.empty()) {
        error_ = "File contains no valid keystrokes";
        return false;
    }

    return true;
}

std::optional<int> FileInputSource::parse_keystroke(const std::string& line) {
    // Handle special keys in angle brackets: <up>, <down>, <left>, <right>
    if (line.size() >= 2 && line[0] == '<' && line[line.size() - 1] == '>') {
        std::string key_name = line.substr(1, line.size() - 2);

        // Convert to lowercase for case-insensitive matching
        std::transform(key_name.begin(), key_name.end(), key_name.begin(),
                       [](unsigned char c) { return std::tolower(c); });

        if (key_name == "up") {
            return Terminal::KEY_UP_ARROW;
        } else if (key_name == "down") {
            return Terminal::KEY_DOWN_ARROW;
        } else if (key_name == "left") {
            return Terminal::KEY_LEFT_ARROW;
        } else if (key_name == "right") {
            return Terminal::KEY_RIGHT_ARROW;
        } else if (key_name == "space") {
            return ' ';
        } else if (key_name == "tab") {
            return '\t';
        } else if (key_name == "enter") {
            return '\n';
        } else if (key_name == "esc") {
            return 27;  // ESC
        } else {
            return std::nullopt;  // Unknown special key
        }
    }

    // Handle regular characters (single character or escape sequences)
    if (line.size() == 1) {
        return static_cast<int>(line[0]);
    }

    // Handle escape sequences like \n, \t, backslash
    if (line.size() == 2 && line[0] == '\\') {
        switch (line[1]) {
            case 'n':
                return '\n';
            case 't':
                return '\t';
            case 'r':
                return '\r';
            case '\\':
                return '\\';
            default:
                return std::nullopt;  // Unknown escape sequence
        }
    }

    return std::nullopt;  // Invalid format
}

}  // namespace datapainter
