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

    // Input handling
    // Enable raw mode (disable line buffering, echo)
    bool enter_raw_mode();
    // Restore normal terminal mode
    bool exit_raw_mode();
    // Read a single character (non-blocking if possible)
    int read_key();

private:
    int rows_;
    int cols_;
    std::vector<std::vector<char>> buffer_;

    void resize_buffer();
};

} // namespace datapainter
