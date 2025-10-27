#pragma once

#include <string>
#include <vector>

namespace datapainter {

// Terminal screen management
class Terminal {
public:
    Terminal();
    ~Terminal();

    // Terminal dimensions
    int rows() const { return rows_; }
    int cols() const { return cols_; }

    // Override dimensions (for testing)
    void set_dimensions(int rows, int cols);

    // Detect actual terminal size
    bool detect_size();

    // Check if dimensions are valid (not too small)
    bool is_size_adequate() const;

    // Screen buffer operations
    void clear_buffer();
    void write_char(int row, int col, char ch);
    char read_char(int row, int col) const;
    std::string get_row(int row) const;

    // Rendering
    void render();  // Output buffer to stdout
    void render_with_cursor(int cursor_row, int cursor_col);  // Render with visible cursor

    // Input handling
    // Enable raw mode (disable line buffering, echo)
    bool enter_raw_mode();
    // Restore normal terminal mode
    bool exit_raw_mode();
    // Read a single character (non-blocking if possible)
    // Returns: character code, or special values for arrow keys:
    //   KEY_UP_ARROW = 1000, KEY_DOWN_ARROW = 1001,
    //   KEY_LEFT_ARROW = 1002, KEY_RIGHT_ARROW = 1003
    int read_key();

    // Special key codes (to avoid conflicts with regular ASCII)
    static constexpr int KEY_UP_ARROW = 1000;
    static constexpr int KEY_DOWN_ARROW = 1001;
    static constexpr int KEY_LEFT_ARROW = 1002;
    static constexpr int KEY_RIGHT_ARROW = 1003;

private:
    int rows_;
    int cols_;
    std::vector<std::vector<char>> buffer_;

    void resize_buffer();
};

} // namespace datapainter
