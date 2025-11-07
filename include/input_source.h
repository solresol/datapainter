#pragma once

#include <string>
#include <vector>
#include <optional>

namespace datapainter {

// Abstract interface for input sources
// Allows switching between terminal input and file-based keystroke playback
class InputSource {
public:
    virtual ~InputSource() = default;

    // Read next keystroke
    // Returns: key code (regular ASCII or special key codes like Terminal::KEY_UP_ARROW)
    // Returns -1 if no more input available (EOF for file sources)
    virtual int read_key() = 0;

    // Check if more input is available
    virtual bool has_input() const = 0;
};

// Terminal-based input source (reads from stdin)
class TerminalInputSource : public InputSource {
public:
    explicit TerminalInputSource(class Terminal& terminal);

    int read_key() override;
    bool has_input() const override;

private:
    Terminal& terminal_;
};

// File-based input source (reads from keystroke file)
class FileInputSource : public InputSource {
public:
    explicit FileInputSource(const std::string& filename);

    int read_key() override;
    bool has_input() const override;

    // Get error message if file loading failed
    std::string get_error() const { return error_; }

private:
    std::vector<int> keystrokes_;
    size_t current_index_;
    std::string error_;

    // Parse keystroke file and populate keystrokes_ vector
    bool parse_file(const std::string& filename);

    // Parse a single line from the keystroke file
    std::optional<int> parse_keystroke(const std::string& line);
};

}  // namespace datapainter
