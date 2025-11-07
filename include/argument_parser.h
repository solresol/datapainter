#pragma once

#include <optional>
#include <string>
#include <vector>

namespace datapainter {

// Parsed command-line arguments
struct Arguments {
    // Database and table
    std::optional<std::string> database;
    std::optional<std::string> table;

    // Axis and column names
    std::optional<std::string> x_axis_name;
    std::optional<std::string> y_axis_name;
    std::optional<std::string> target_column_name;

    // Target meanings
    std::optional<std::string> x_meaning;
    std::optional<std::string> o_meaning;

    // Valid ranges
    std::optional<double> min_x;
    std::optional<double> max_x;
    std::optional<double> min_y;
    std::optional<double> max_y;

    // UI options
    bool show_zero_bars = false;
    std::optional<int> override_screen_height;
    std::optional<int> override_screen_width;
    bool start_tabular = false;

    // Non-interactive mode commands
    bool create_table = false;
    bool rename_table = false;
    bool copy_table = false;
    bool delete_table = false;
    bool list_tables = false;
    bool show_metadata = false;
    bool add_point = false;
    bool delete_point = false;
    bool to_csv = false;

    // Point operation arguments
    std::optional<double> point_x;
    std::optional<double> point_y;
    std::optional<std::string> point_target;
    std::optional<int> point_id;

    // Testing/debug options
    bool dump_screen = false;
    bool dump_edit_area_contents = false;
    std::optional<std::string> key_stroke_at_point;
    bool zoom_in = false;
    bool zoom_out = false;
    bool list_x_axis_marks = false;
    bool list_y_axis_marks = false;
    std::optional<std::string> keystroke_file;

    // Study mode
    bool study = false;

    // Random initialization
    std::optional<int> random_count;
    std::optional<std::string> random_target;
    std::optional<double> mean_x;
    std::optional<double> mean_y;
    bool normal_x = false;
    bool normal_y = false;
    std::optional<double> std_x;
    std::optional<double> std_y;
    bool uniform_x = false;
    bool uniform_y = false;
    std::optional<double> range_x;
    std::optional<double> range_y;

    // Undo log operations
    bool clear_undo_log = false;
    bool clear_all_undo_log = false;
    bool commit_unsaved_changes = false;
    bool list_unsaved_changes = false;

    // Help and version
    bool show_help = false;
    bool show_version = false;

    // Validation
    bool has_errors() const { return !error_messages.empty(); }
    std::vector<std::string> error_messages;
};

// Parse command-line arguments
class ArgumentParser {
public:
    // Parse argc/argv into Arguments struct
    static Arguments parse(int argc, char** argv);

    // Validate that parsed arguments are consistent
    // Returns error messages if validation fails
    static std::vector<std::string> validate(const Arguments& args);

    // Detect conflicts between CLI arguments and existing metadata
    // Returns error messages for any conflicts found
    static std::vector<std::string> detect_conflicts(const Arguments& args, const struct Metadata& metadata);

    // Print help message to the given output stream
    static void print_help(std::ostream& out);

private:
    // Helper to check if a flag is present
    static bool has_flag(int argc, char** argv, const std::string& flag);

    // Helper to get value after a flag
    static std::optional<std::string> get_value(int argc, char** argv, const std::string& flag);

    // Helper to parse double value
    static std::optional<double> parse_double(const std::string& str);

    // Helper to parse int value
    static std::optional<int> parse_int(const std::string& str);
};

} // namespace datapainter
