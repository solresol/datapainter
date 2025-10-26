#include "argument_parser.h"
#include <algorithm>
#include <sstream>

namespace datapainter {

Arguments ArgumentParser::parse(int argc, char** argv) {
    Arguments args;

    // Database and table
    args.database = get_value(argc, argv, "--database");
    args.table = get_value(argc, argv, "--table");

    // Axis and column names
    args.x_axis_name = get_value(argc, argv, "--x-axis-name");
    args.y_axis_name = get_value(argc, argv, "--y-axis-name");
    args.target_column_name = get_value(argc, argv, "--target-column-name");

    // Target meanings
    args.x_meaning = get_value(argc, argv, "--x-meaning");
    args.o_meaning = get_value(argc, argv, "--o-meaning");

    // Valid ranges
    if (auto val = get_value(argc, argv, "--min-x")) {
        if (auto parsed = parse_double(*val)) {
            args.min_x = *parsed;
        } else {
            args.error_messages.push_back("Invalid value for --min-x: " + *val);
        }
    }

    if (auto val = get_value(argc, argv, "--max-x")) {
        if (auto parsed = parse_double(*val)) {
            args.max_x = *parsed;
        } else {
            args.error_messages.push_back("Invalid value for --max-x: " + *val);
        }
    }

    if (auto val = get_value(argc, argv, "--min-y")) {
        if (auto parsed = parse_double(*val)) {
            args.min_y = *parsed;
        } else {
            args.error_messages.push_back("Invalid value for --min-y: " + *val);
        }
    }

    if (auto val = get_value(argc, argv, "--max-y")) {
        if (auto parsed = parse_double(*val)) {
            args.max_y = *parsed;
        } else {
            args.error_messages.push_back("Invalid value for --max-y: " + *val);
        }
    }

    // UI options
    args.show_zero_bars = has_flag(argc, argv, "--show-zero-bars");
    args.start_tabular = has_flag(argc, argv, "--start-tabular");

    if (auto val = get_value(argc, argv, "--override-screen-height")) {
        if (auto parsed = parse_int(*val)) {
            args.override_screen_height = *parsed;
        } else {
            args.error_messages.push_back("Invalid value for --override-screen-height: " + *val);
        }
    }

    if (auto val = get_value(argc, argv, "--override-screen-width")) {
        if (auto parsed = parse_int(*val)) {
            args.override_screen_width = *parsed;
        } else {
            args.error_messages.push_back("Invalid value for --override-screen-width: " + *val);
        }
    }

    // Non-interactive mode commands
    args.create_table = has_flag(argc, argv, "--create-table");
    args.rename_table = has_flag(argc, argv, "--rename-table");
    args.copy_table = has_flag(argc, argv, "--copy-table");
    args.delete_table = has_flag(argc, argv, "--delete-table");
    args.list_tables = has_flag(argc, argv, "--list-tables");
    args.show_metadata = has_flag(argc, argv, "--show-metadata");
    args.add_point = has_flag(argc, argv, "--add-point");
    args.delete_point = has_flag(argc, argv, "--delete-point");
    args.to_csv = has_flag(argc, argv, "--to-csv");

    // Testing/debug options
    args.dump_screen = has_flag(argc, argv, "--dump-screen");
    args.dump_edit_area_contents = has_flag(argc, argv, "--dump-edit-area-contents");
    args.key_stroke_at_point = get_value(argc, argv, "--key-stroke-at-point");
    args.zoom_in = has_flag(argc, argv, "--zoom-in");
    args.zoom_out = has_flag(argc, argv, "--zoom-out");
    args.list_x_axis_marks = has_flag(argc, argv, "--list-x-axis-marks");
    args.list_y_axis_marks = has_flag(argc, argv, "--list-y-axis-marks");

    // Study mode
    args.study = has_flag(argc, argv, "--study");

    // Random initialization
    if (auto val = get_value(argc, argv, "--random-count")) {
        if (auto parsed = parse_int(*val)) {
            args.random_count = *parsed;
        } else {
            args.error_messages.push_back("Invalid value for --random-count: " + *val);
        }
    }

    args.random_target = get_value(argc, argv, "--random-target");

    if (auto val = get_value(argc, argv, "--mean-x")) {
        if (auto parsed = parse_double(*val)) {
            args.mean_x = *parsed;
        } else {
            args.error_messages.push_back("Invalid value for --mean-x: " + *val);
        }
    }

    if (auto val = get_value(argc, argv, "--mean-y")) {
        if (auto parsed = parse_double(*val)) {
            args.mean_y = *parsed;
        } else {
            args.error_messages.push_back("Invalid value for --mean-y: " + *val);
        }
    }

    args.normal_x = has_flag(argc, argv, "--normal-x");
    args.normal_y = has_flag(argc, argv, "--normal-y");
    args.uniform_x = has_flag(argc, argv, "--uniform-x");
    args.uniform_y = has_flag(argc, argv, "--uniform-y");

    if (auto val = get_value(argc, argv, "--std-x")) {
        if (auto parsed = parse_double(*val)) {
            args.std_x = *parsed;
        } else {
            args.error_messages.push_back("Invalid value for --std-x: " + *val);
        }
    }

    if (auto val = get_value(argc, argv, "--std-y")) {
        if (auto parsed = parse_double(*val)) {
            args.std_y = *parsed;
        } else {
            args.error_messages.push_back("Invalid value for --std-y: " + *val);
        }
    }

    if (auto val = get_value(argc, argv, "--range-x")) {
        if (auto parsed = parse_double(*val)) {
            args.range_x = *parsed;
        } else {
            args.error_messages.push_back("Invalid value for --range-x: " + *val);
        }
    }

    if (auto val = get_value(argc, argv, "--range-y")) {
        if (auto parsed = parse_double(*val)) {
            args.range_y = *parsed;
        } else {
            args.error_messages.push_back("Invalid value for --range-y: " + *val);
        }
    }

    // Undo log operations
    args.clear_undo_log = has_flag(argc, argv, "--clear-undo-log");
    args.clear_all_undo_log = has_flag(argc, argv, "--clear-all-undo-log");
    args.commit_unsaved_changes = has_flag(argc, argv, "--commit-unsaved-changes");
    args.list_unsaved_changes = has_flag(argc, argv, "--list-unsaved-changes");

    return args;
}

std::vector<std::string> ArgumentParser::validate(const Arguments& args) {
    std::vector<std::string> errors;

    // Validate min <= max for x range
    if (args.min_x.has_value() && args.max_x.has_value()) {
        if (args.min_x.value() > args.max_x.value()) {
            errors.push_back("Invalid range: min_x (" + std::to_string(args.min_x.value()) +
                           ") must be <= max_x (" + std::to_string(args.max_x.value()) + ")");
        }
    }

    // Validate min <= max for y range
    if (args.min_y.has_value() && args.max_y.has_value()) {
        if (args.min_y.value() > args.max_y.value()) {
            errors.push_back("Invalid range: min_y (" + std::to_string(args.min_y.value()) +
                           ") must be <= max_y (" + std::to_string(args.max_y.value()) + ")");
        }
    }

    return errors;
}

bool ArgumentParser::has_flag(int argc, char** argv, const std::string& flag) {
    for (int i = 1; i < argc; i++) {
        if (argv[i] == flag) {
            return true;
        }
    }
    return false;
}

std::optional<std::string> ArgumentParser::get_value(int argc, char** argv, const std::string& flag) {
    for (int i = 1; i < argc - 1; i++) {
        if (argv[i] == flag) {
            return std::string(argv[i + 1]);
        }
    }
    return std::nullopt;
}

std::optional<double> ArgumentParser::parse_double(const std::string& str) {
    try {
        size_t pos;
        double value = std::stod(str, &pos);
        // Check that entire string was consumed
        if (pos == str.length()) {
            return value;
        }
    } catch (...) {
        // Parsing failed
    }
    return std::nullopt;
}

std::optional<int> ArgumentParser::parse_int(const std::string& str) {
    try {
        size_t pos;
        int value = std::stoi(str, &pos);
        // Check that entire string was consumed
        if (pos == str.length()) {
            return value;
        }
    } catch (...) {
        // Parsing failed
    }
    return std::nullopt;
}

} // namespace datapainter
