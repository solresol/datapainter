#include "table_creation_dialog.h"
#include <algorithm>
#include <cctype>
#include <sstream>

namespace datapainter {

TableCreationDialog::TableCreationDialog(Terminal& terminal)
    : terminal_(terminal), current_field_(0), cursor_pos_(0) {
    init_fields();
}

void TableCreationDialog::init_fields() {
    fields_ = {
        {"Table name", "", "", false},
        {"Target column name (e.g., 'label', 'class')", "", "", false},
        {"X-axis name (e.g., 'x', 'feature1')", "", "", false},
        {"Y-axis name (e.g., 'y', 'feature2')", "", "", false},
        {"X meaning (label for 'x' points)", "", "", false},
        {"O meaning (label for 'o' points)", "", "", false},
        {"Min X", "", "-10.0", true},
        {"Max X", "", "10.0", true},
        {"Min Y", "", "-10.0", true},
        {"Max Y", "", "10.0", true}
    };
}

TableCreationResult TableCreationDialog::run() {
    TableCreationResult result;
    result.cancelled = false;

    terminal_.enter_raw_mode();

    bool done = false;
    bool ok_pressed = false;

    while (!done) {
        render();

        int key = terminal_.read_key();

        // Handle arrow keys
        if (key == Terminal::KEY_UP_ARROW) {
            prev_field();
        } else if (key == Terminal::KEY_DOWN_ARROW) {
            next_field();
        } else if (key == Terminal::KEY_LEFT_ARROW) {
            move_cursor_left();
        } else if (key == Terminal::KEY_RIGHT_ARROW) {
            move_cursor_right();
        } else if (key == '\t' || key == '\n') {  // Tab or Enter
            next_field();
        } else if (key == 127 || key == '\b') {  // Backspace
            delete_char();
        } else if (key == '\x0f') {  // Ctrl+O - OK
            if (validate_input()) {
                ok_pressed = true;
                done = true;
            }
        } else if (key == '\x1b' || key == '\x03' || key == '\x11') {  // ESC, Ctrl+C or Ctrl+Q
            result.cancelled = true;
            done = true;
        } else if (key >= 32 && key < 127) {  // Printable ASCII
            insert_char(static_cast<char>(key));
        }
    }

    terminal_.exit_raw_mode();
    terminal_.clear_buffer();
    terminal_.render();

    if (!ok_pressed || result.cancelled) {
        result.cancelled = true;
        return result;
    }

    // Fill in the result
    result.table_name = fields_[0].value;
    result.target_col = fields_[1].value;
    result.x_axis = fields_[2].value;
    result.y_axis = fields_[3].value;
    result.x_meaning = fields_[4].value;
    result.o_meaning = fields_[5].value;

    // Parse numeric fields with defaults
    if (!try_parse_double(fields_[6].value.empty() ? fields_[6].default_value : fields_[6].value,
                          result.min_x)) {
        result.min_x = -10.0;
    }
    if (!try_parse_double(fields_[7].value.empty() ? fields_[7].default_value : fields_[7].value,
                          result.max_x)) {
        result.max_x = 10.0;
    }
    if (!try_parse_double(fields_[8].value.empty() ? fields_[8].default_value : fields_[8].value,
                          result.min_y)) {
        result.min_y = -10.0;
    }
    if (!try_parse_double(fields_[9].value.empty() ? fields_[9].default_value : fields_[9].value,
                          result.max_y)) {
        result.max_y = 10.0;
    }

    return result;
}

void TableCreationDialog::render() {
    terminal_.clear_buffer();

    int screen_height = terminal_.rows();
    int screen_width = terminal_.cols();

    // Title
    std::string title = "CREATE NEW TABLE";
    int title_col = (screen_width - static_cast<int>(title.length())) / 2;
    if (title_col < 0) title_col = 0;

    for (size_t i = 0; i < title.length() && title_col + static_cast<int>(i) < screen_width; ++i) {
        terminal_.write_char(0, title_col + static_cast<int>(i), title[i]);
    }

    // Draw separator
    for (int col = 0; col < screen_width; ++col) {
        terminal_.write_char(1, col, '=');
    }

    // Render each field
    int y = 3;
    for (size_t i = 0; i < fields_.size(); ++i) {
        render_field(y, fields_[i], i == current_field_);
        y += 3;  // Each field takes 3 rows (label, value, blank)
    }

    // Instructions
    y += 1;
    if (y < screen_height - 3) {
        for (int col = 0; col < screen_width; ++col) {
            terminal_.write_char(y, col, '-');
        }
        y++;

        std::string help = "Up/Down: Navigate | Ctrl+O: OK | ESC: Cancel";
        int help_col = 2;
        for (size_t i = 0; i < help.length() && help_col + static_cast<int>(i) < screen_width; ++i) {
            terminal_.write_char(y, help_col + static_cast<int>(i), help[i]);
        }
        y++;
    }

    // Error message
    if (!error_message_.empty() && y < screen_height) {
        std::string err = "ERROR: " + error_message_;
        for (size_t i = 0; i < err.length() && static_cast<int>(i) < screen_width; ++i) {
            terminal_.write_char(y, static_cast<int>(i), err[i]);
        }
    }

    // Render to screen with cursor positioned in current field
    int cursor_row = 4 + current_field_ * 3;  // Field value row
    int cursor_col = 3 + cursor_pos_;  // After "  ["
    if (cursor_row < screen_height && cursor_col < screen_width) {
        terminal_.render_with_cursor(cursor_row, cursor_col);
    } else {
        terminal_.render();
    }
}

void TableCreationDialog::render_field(int y, const Field& field, bool /*is_active*/) {
    int screen_width = terminal_.cols();

    // Label
    std::string label = field.label + ":";
    for (size_t i = 0; i < label.length() && static_cast<int>(i) < screen_width; ++i) {
        terminal_.write_char(y, static_cast<int>(i), label[i]);
    }

    // Value field
    std::string value_display = field.value;
    if (value_display.empty() && !field.default_value.empty()) {
        value_display = field.default_value;
    }

    // Draw field box
    terminal_.write_char(y + 1, 0, ' ');
    terminal_.write_char(y + 1, 1, ' ');
    terminal_.write_char(y + 1, 2, '[');

    // Write value
    for (size_t i = 0; i < value_display.length() && 3 + static_cast<int>(i) < screen_width - 1; ++i) {
        terminal_.write_char(y + 1, 3 + static_cast<int>(i), value_display[i]);
    }

    // Pad with spaces
    for (size_t i = value_display.length(); i < 60 && 3 + static_cast<int>(i) < screen_width - 1; ++i) {
        terminal_.write_char(y + 1, 3 + static_cast<int>(i), ' ');
    }

    // Closing bracket
    int bracket_col = std::min(63, screen_width - 1);
    terminal_.write_char(y + 1, bracket_col, ']');
}

void TableCreationDialog::insert_char(char c) {
    Field& field = fields_[current_field_];

    if (cursor_pos_ <= field.value.length() && field.value.length() < 60) {
        field.value.insert(cursor_pos_, 1, c);
        cursor_pos_++;
        error_message_.clear();
    }
}

void TableCreationDialog::delete_char() {
    Field& field = fields_[current_field_];

    if (cursor_pos_ > 0 && cursor_pos_ <= field.value.length()) {
        field.value.erase(cursor_pos_ - 1, 1);
        cursor_pos_--;
        error_message_.clear();
    }
}

void TableCreationDialog::move_cursor_left() {
    if (cursor_pos_ > 0) {
        cursor_pos_--;
    }
}

void TableCreationDialog::move_cursor_right() {
    Field& field = fields_[current_field_];
    if (cursor_pos_ < field.value.length()) {
        cursor_pos_++;
    }
}

void TableCreationDialog::next_field() {
    if (current_field_ < fields_.size() - 1) {
        current_field_++;
        cursor_pos_ = fields_[current_field_].value.length();
    }
}

void TableCreationDialog::prev_field() {
    if (current_field_ > 0) {
        current_field_--;
        cursor_pos_ = fields_[current_field_].value.length();
    }
}

bool TableCreationDialog::validate_input() {
    // Check required string fields
    if (fields_[0].value.empty()) {
        error_message_ = "Table name is required";
        current_field_ = 0;
        cursor_pos_ = 0;
        return false;
    }

    if (fields_[1].value.empty()) {
        error_message_ = "Target column name is required";
        current_field_ = 1;
        cursor_pos_ = 0;
        return false;
    }

    if (fields_[2].value.empty()) {
        error_message_ = "X-axis name is required";
        current_field_ = 2;
        cursor_pos_ = 0;
        return false;
    }

    if (fields_[3].value.empty()) {
        error_message_ = "Y-axis name is required";
        current_field_ = 3;
        cursor_pos_ = 0;
        return false;
    }

    if (fields_[4].value.empty()) {
        error_message_ = "X meaning is required";
        current_field_ = 4;
        cursor_pos_ = 0;
        return false;
    }

    if (fields_[5].value.empty()) {
        error_message_ = "O meaning is required";
        current_field_ = 5;
        cursor_pos_ = 0;
        return false;
    }

    // Validate numeric fields
    double min_x, max_x, min_y, max_y;

    std::string min_x_str = fields_[6].value.empty() ? fields_[6].default_value : fields_[6].value;
    if (!try_parse_double(min_x_str, min_x)) {
        error_message_ = "Min X must be a valid number";
        current_field_ = 6;
        cursor_pos_ = 0;
        return false;
    }

    std::string max_x_str = fields_[7].value.empty() ? fields_[7].default_value : fields_[7].value;
    if (!try_parse_double(max_x_str, max_x)) {
        error_message_ = "Max X must be a valid number";
        current_field_ = 7;
        cursor_pos_ = 0;
        return false;
    }

    std::string min_y_str = fields_[8].value.empty() ? fields_[8].default_value : fields_[8].value;
    if (!try_parse_double(min_y_str, min_y)) {
        error_message_ = "Min Y must be a valid number";
        current_field_ = 8;
        cursor_pos_ = 0;
        return false;
    }

    std::string max_y_str = fields_[9].value.empty() ? fields_[9].default_value : fields_[9].value;
    if (!try_parse_double(max_y_str, max_y)) {
        error_message_ = "Max Y must be a valid number";
        current_field_ = 9;
        cursor_pos_ = 0;
        return false;
    }

    // Validate ranges
    if (min_x >= max_x) {
        error_message_ = "Min X must be less than Max X";
        current_field_ = 6;
        cursor_pos_ = 0;
        return false;
    }

    if (min_y >= max_y) {
        error_message_ = "Min Y must be less than Max Y";
        current_field_ = 8;
        cursor_pos_ = 0;
        return false;
    }

    error_message_.clear();
    return true;
}

bool TableCreationDialog::try_parse_double(const std::string& str, double& out) {
    try {
        size_t pos;
        out = std::stod(str, &pos);
        // Check if entire string was consumed
        return pos == str.length();
    } catch (...) {
        return false;
    }
}

}  // namespace datapainter
