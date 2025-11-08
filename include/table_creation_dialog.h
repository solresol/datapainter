#pragma once

#include "terminal.h"
#include <string>
#include <vector>

namespace datapainter {

struct TableCreationResult {
    bool cancelled;
    std::string table_name;
    std::string target_col;
    std::string x_axis;
    std::string y_axis;
    std::string x_meaning;
    std::string o_meaning;
    double min_x;
    double max_x;
    double min_y;
    double max_y;
};

class TableCreationDialog {
public:
    explicit TableCreationDialog(Terminal& terminal);

    // Run the dialog and return the result
    TableCreationResult run();

private:
    struct Field {
        std::string label;
        std::string value;
        std::string default_value;
        bool is_numeric;
    };

    Terminal& terminal_;
    std::vector<Field> fields_;
    size_t current_field_;
    size_t cursor_pos_;
    std::string error_message_;

    void init_fields();
    void render();
    void render_field(int y, const Field& field, bool is_active);
    void handle_input(char key);
    void insert_char(char c);
    void delete_char();
    void move_cursor_left();
    void move_cursor_right();
    void next_field();
    void prev_field();
    bool validate_input();
    bool try_parse_double(const std::string& str, double& out);
};

}  // namespace datapainter
