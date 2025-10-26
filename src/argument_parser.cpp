#include "argument_parser.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace datapainter {

Arguments ArgumentParser::parse(int argc, char** argv) {
    Arguments args;

    // Check for help and version flags first
    args.show_help = has_flag(argc, argv, "--help") || has_flag(argc, argv, "-h");
    args.show_version = has_flag(argc, argv, "--version");

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

    // Point operation arguments
    if (auto val = get_value(argc, argv, "--x")) {
        if (auto parsed = parse_double(*val)) {
            args.point_x = *parsed;
        } else {
            args.error_messages.push_back("Invalid value for --x: " + *val);
        }
    }

    if (auto val = get_value(argc, argv, "--y")) {
        if (auto parsed = parse_double(*val)) {
            args.point_y = *parsed;
        } else {
            args.error_messages.push_back("Invalid value for --y: " + *val);
        }
    }

    args.point_target = get_value(argc, argv, "--target");

    if (auto val = get_value(argc, argv, "--point-id")) {
        if (auto parsed = parse_int(*val)) {
            args.point_id = *parsed;
        } else {
            args.error_messages.push_back("Invalid value for --point-id: " + *val);
        }
    }

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

void ArgumentParser::print_help(std::ostream& out) {
    out << "DataPainter v0.1.0 - TUI for editing 2D labeled datasets\n\n";

    out << "USAGE:\n";
    out << "  datapainter [OPTIONS]\n\n";

    out << "GENERAL OPTIONS:\n";
    out << "  --help, -h              Show this help message\n";
    out << "  --version               Show version information\n";
    out << "  --database <path>       Path to SQLite database file\n";
    out << "  --table <name>          Table name to work with\n\n";

    out << "TABLE MANAGEMENT:\n";
    out << "  --list-tables           List all tables in the database\n";
    out << "  --create-table          Create a new table (requires additional options)\n";
    out << "  --delete-table          Delete a table\n";
    out << "  --rename-table          Rename a table (not yet implemented)\n";
    out << "  --copy-table            Copy a table (not yet implemented)\n";
    out << "  --show-metadata         Show metadata for a table\n\n";

    out << "CREATE TABLE OPTIONS:\n";
    out << "  --target-column-name <name>  Name for target/label column\n";
    out << "  --x-axis-name <name>         Name for X axis\n";
    out << "  --y-axis-name <name>         Name for Y axis\n";
    out << "  --x-meaning <char>           Character representing X axis targets\n";
    out << "  --o-meaning <char>           Character representing O/other targets\n";
    out << "  --min-x <value>              Minimum X value (default: -10.0)\n";
    out << "  --max-x <value>              Maximum X value (default: 10.0)\n";
    out << "  --min-y <value>              Minimum Y value (default: -10.0)\n";
    out << "  --max-y <value>              Maximum Y value (default: 10.0)\n";
    out << "  --show-zero-bars             Show zero axis bars\n\n";

    out << "POINT OPERATIONS:\n";
    out << "  --add-point             Add a point (requires --x, --y, --target)\n";
    out << "  --delete-point          Delete a point (requires --point-id)\n";
    out << "  --x <value>             X coordinate for point\n";
    out << "  --y <value>             Y coordinate for point\n";
    out << "  --target <label>        Target/label for point\n";
    out << "  --point-id <id>         ID of point to delete\n\n";

    out << "DATA EXPORT:\n";
    out << "  --to-csv                Export table data to CSV format\n\n";

    out << "UNDO LOG MANAGEMENT:\n";
    out << "  --list-unsaved-changes  List all unsaved changes for a table\n";
    out << "  --commit-unsaved-changes Commit unsaved changes for a table\n";
    out << "  --clear-undo-log        Clear undo log for a table\n";
    out << "  --clear-all-undo-log    Clear undo logs for all tables\n\n";

    out << "UI OPTIONS (for interactive mode):\n";
    out << "  --start-tabular         Start in tabular view mode\n";
    out << "  --override-screen-width <cols>   Override detected screen width\n";
    out << "  --override-screen-height <rows>  Override detected screen height\n\n";

    out << "DEBUG OPTIONS:\n";
    out << "  --dump-screen           Dump screen buffer contents\n";
    out << "  --dump-edit-area-contents  Dump edit area contents\n";
    out << "  --list-x-axis-marks     List X axis tick marks\n";
    out << "  --list-y-axis-marks     List Y axis tick marks\n";
    out << "  --zoom-in               Zoom in\n";
    out << "  --zoom-out              Zoom out\n\n";

    out << "EXAMPLES:\n";
    out << "  # Create a new table\n";
    out << "  datapainter --database data.db --create-table --table mytable \\\n";
    out << "    --target-column-name label --x-axis-name x --y-axis-name y \\\n";
    out << "    --x-meaning X --o-meaning O\n\n";

    out << "  # Add a point\n";
    out << "  datapainter --database data.db --table mytable --add-point \\\n";
    out << "    --x 1.5 --y 2.3 --target positive\n\n";

    out << "  # Export to CSV\n";
    out << "  datapainter --database data.db --table mytable --to-csv > output.csv\n\n";

    out << "  # List all tables\n";
    out << "  datapainter --database data.db --list-tables\n\n";

    out << "For more information, see README.md\n";
}

} // namespace datapainter
