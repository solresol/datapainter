#include "argument_parser.h"
#include "database.h"
#include "table_manager.h"
#include "undo_log_manager.h"
#include "data_table.h"
#include "terminal.h"
#include "viewport.h"
#include "metadata.h"
#include "header_renderer.h"
#include "footer_renderer.h"
#include "edit_area_renderer.h"
#include "table_selection_menu.h"
#include "point_editor.h"
#include "unsaved_changes.h"
#include "save_manager.h"
#include "help_overlay.h"
#include "cursor_utils.h"
#include <algorithm>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <limits>

using namespace datapainter;

int main(int argc, char** argv) {
    // Parse arguments
    Arguments args = ArgumentParser::parse(argc, argv);

    // Handle --help flag
    if (args.show_help) {
        ArgumentParser::print_help(std::cout);
        return 0;
    }

    // Handle --version flag
    if (args.show_version) {
        std::cout << "DataPainter v0.1.0" << std::endl;
        return 0;
    }

    // Check for parsing errors
    if (args.has_errors()) {
        for (const auto& error : args.error_messages) {
            std::cerr << "Error: " << error << std::endl;
        }
        return 2;
    }

    // Validate arguments
    auto validation_errors = ArgumentParser::validate(args);
    if (!validation_errors.empty()) {
        for (const auto& error : validation_errors) {
            std::cerr << "Error: " << error << std::endl;
        }
        return 2;
    }

    // Check if database is required
    bool needs_database = args.create_table || args.rename_table || args.copy_table ||
                          args.delete_table || args.list_tables || args.show_metadata ||
                          args.add_point || args.delete_point || args.to_csv ||
                          args.clear_undo_log || args.clear_all_undo_log ||
                          args.commit_unsaved_changes || args.list_unsaved_changes;

    if (needs_database && !args.database.has_value()) {
        std::cerr << "Error: --database is required for this operation" << std::endl;
        return 2;
    }

    // Open database if needed
    if (!args.database.has_value()) {
        // No database specified - show help
        std::cout << "DataPainter v0.1.0 - TUI for editing 2D labeled datasets\n" << std::endl;
        std::cout << "No database specified. Use --database <path> to open a database." << std::endl;
        std::cout << "For full help, run: datapainter --help\n" << std::endl;
        std::cout << "Quick start:" << std::endl;
        std::cout << "  datapainter --database data.db --list-tables" << std::endl;
        std::cout << "  datapainter --database data.db --table mytable  (interactive mode)" << std::endl;
        return 0;
    }

    Database db(args.database.value());
    if (!db.is_open()) {
        std::cerr << "Error: Failed to open database: " << args.database.value() << std::endl;
        return 65;
    }

    // Ensure system tables exist
    if (!db.ensure_metadata_table() || !db.ensure_unsaved_changes_table()) {
        std::cerr << "Error: Failed to create system tables" << std::endl;
        return 66;
    }

    // Handle non-interactive commands
    TableManager table_mgr(db);
    UndoLogManager undo_mgr(db);

    // --list-tables
    if (args.list_tables) {
        auto tables = table_mgr.list_tables();
        if (tables.empty()) {
            std::cout << "No tables found in database" << std::endl;
        } else {
            std::cout << "Tables:" << std::endl;
            for (const auto& table : tables) {
                std::cout << "  " << table << std::endl;
            }
        }
        return 0;
    }

    // --create-table
    if (args.create_table) {
        if (!args.table.has_value()) {
            std::cerr << "Error: --table is required for --create-table" << std::endl;
            return 2;
        }
        if (!args.target_column_name.has_value()) {
            std::cerr << "Error: --target-column-name is required for --create-table" << std::endl;
            return 2;
        }
        if (!args.x_axis_name.has_value()) {
            std::cerr << "Error: --x-axis-name is required for --create-table" << std::endl;
            return 2;
        }
        if (!args.y_axis_name.has_value()) {
            std::cerr << "Error: --y-axis-name is required for --create-table" << std::endl;
            return 2;
        }
        if (!args.x_meaning.has_value()) {
            std::cerr << "Error: --x-meaning is required for --create-table" << std::endl;
            return 2;
        }
        if (!args.o_meaning.has_value()) {
            std::cerr << "Error: --o-meaning is required for --create-table" << std::endl;
            return 2;
        }

        bool success = table_mgr.create_table(
            args.table.value(),
            args.target_column_name.value(),
            args.x_axis_name.value(),
            args.y_axis_name.value(),
            args.x_meaning.value(),
            args.o_meaning.value(),
            args.min_x.value_or(-10.0),
            args.max_x.value_or(10.0),
            args.min_y.value_or(-10.0),
            args.max_y.value_or(10.0),
            args.show_zero_bars
        );

        if (!success) {
            std::cerr << "Error: Failed to create table" << std::endl;
            return 66;
        }

        std::cout << "Table '" << args.table.value() << "' created successfully" << std::endl;
        return 0;
    }

    // --show-metadata
    if (args.show_metadata) {
        if (!args.table.has_value()) {
            std::cerr << "Error: --table is required for --show-metadata" << std::endl;
            return 2;
        }

        if (!table_mgr.show_metadata(args.table.value(), std::cout)) {
            std::cerr << "Error: Table not found: " << args.table.value() << std::endl;
            return 66;
        }

        return 0;
    }

    // --list-unsaved-changes
    if (args.list_unsaved_changes) {
        if (!args.table.has_value()) {
            std::cerr << "Error: --table is required for --list-unsaved-changes" << std::endl;
            return 2;
        }

        if (!undo_mgr.list_unsaved_changes(args.table.value(), std::cout)) {
            std::cerr << "Error: Failed to list unsaved changes" << std::endl;
            return 66;
        }

        return 0;
    }

    // --delete-table
    if (args.delete_table) {
        if (!args.table.has_value()) {
            std::cerr << "Error: --table is required for --delete-table" << std::endl;
            return 2;
        }

        if (!table_mgr.delete_table(args.table.value())) {
            std::cerr << "Error: Failed to delete table" << std::endl;
            return 66;
        }

        std::cout << "Table '" << args.table.value() << "' deleted successfully" << std::endl;
        return 0;
    }

    // --rename-table
    if (args.rename_table) {
        // TODO: Need to add --new-table argument
        std::cerr << "Error: --rename-table not yet implemented" << std::endl;
        return 2;
    }

    // --copy-table
    if (args.copy_table) {
        // TODO: Need to add --destination-table argument
        std::cerr << "Error: --copy-table not yet implemented" << std::endl;
        return 2;
    }

    // --add-point
    if (args.add_point) {
        if (!args.table.has_value()) {
            std::cerr << "Error: --table is required for --add-point" << std::endl;
            return 2;
        }
        if (!args.point_x.has_value()) {
            std::cerr << "Error: --x is required for --add-point" << std::endl;
            return 2;
        }
        if (!args.point_y.has_value()) {
            std::cerr << "Error: --y is required for --add-point" << std::endl;
            return 2;
        }
        if (!args.point_target.has_value()) {
            std::cerr << "Error: --target is required for --add-point" << std::endl;
            return 2;
        }

        DataTable dt(db, args.table.value());
        auto id = dt.insert_point(args.point_x.value(), args.point_y.value(), args.point_target.value());

        if (!id.has_value()) {
            std::cerr << "Error: Failed to add point" << std::endl;
            return 66;
        }

        std::cout << "Point added with ID " << id.value() << std::endl;
        return 0;
    }

    // --delete-point
    if (args.delete_point) {
        if (!args.table.has_value()) {
            std::cerr << "Error: --table is required for --delete-point" << std::endl;
            return 2;
        }
        if (!args.point_id.has_value()) {
            std::cerr << "Error: --point-id is required for --delete-point" << std::endl;
            return 2;
        }

        DataTable dt(db, args.table.value());
        if (!dt.delete_point(args.point_id.value())) {
            std::cerr << "Error: Point not found: " << args.point_id.value() << std::endl;
            return 66;
        }

        std::cout << "Point " << args.point_id.value() << " deleted successfully" << std::endl;
        return 0;
    }

    // --clear-undo-log
    if (args.clear_undo_log) {
        if (!args.table.has_value()) {
            std::cerr << "Error: --table is required for --clear-undo-log" << std::endl;
            return 2;
        }

        if (!undo_mgr.clear_undo_log(args.table.value())) {
            std::cerr << "Error: Failed to clear undo log" << std::endl;
            return 66;
        }

        std::cout << "Undo log cleared for table '" << args.table.value() << "'" << std::endl;
        return 0;
    }

    // --clear-all-undo-log
    if (args.clear_all_undo_log) {
        if (!undo_mgr.clear_all_undo_logs()) {
            std::cerr << "Error: Failed to clear all undo logs" << std::endl;
            return 66;
        }

        std::cout << "All undo logs cleared" << std::endl;
        return 0;
    }

    // --commit-unsaved-changes
    if (args.commit_unsaved_changes) {
        if (!args.table.has_value()) {
            std::cerr << "Error: --table is required for --commit-unsaved-changes" << std::endl;
            return 2;
        }

        if (!undo_mgr.commit_unsaved_changes(args.table.value())) {
            std::cerr << "Error: Failed to commit unsaved changes" << std::endl;
            return 66;
        }

        std::cout << "Unsaved changes committed for table '" << args.table.value() << "'" << std::endl;
        return 0;
    }

    // --to-csv
    if (args.to_csv) {
        if (!args.table.has_value()) {
            std::cerr << "Error: --table is required for --to-csv" << std::endl;
            return 2;
        }

        // Query all points from the table
        DataTable dt(db, args.table.value());
        // Use a very large viewport to get all points
        auto points = dt.query_viewport(-1e308, 1e308, -1e308, 1e308);

        // Sort by id to ensure consistent order
        std::sort(points.begin(), points.end(), [](const DataPoint& a, const DataPoint& b) {
            return a.id < b.id;
        });

        // Output CSV header
        std::cout << "x,y,target\n";

        // Output data rows
        for (const auto& point : points) {
            std::cout << point.x << "," << point.y << ",";

            // Escape target value if it contains special characters
            std::string target = point.target;
            bool needs_quotes = target.find(',') != std::string::npos ||
                                target.find('"') != std::string::npos ||
                                target.find('\n') != std::string::npos;

            if (needs_quotes) {
                std::cout << "\"";
                for (char c : target) {
                    if (c == '"') {
                        std::cout << "\"\"";  // Escape quotes by doubling them
                    } else {
                        std::cout << c;
                    }
                }
                std::cout << "\"";
            } else {
                std::cout << target;
            }

            std::cout << "\n";
        }

        return 0;
    }

    // --dump-screen or --dump-edit-area-contents
    if (args.dump_screen || args.dump_edit_area_contents) {
        if (!args.table.has_value()) {
            std::cerr << "Error: --table is required for --dump-screen/--dump-edit-area-contents" << std::endl;
            return 2;
        }

        // Load metadata
        MetadataManager metadata_mgr(db);
        auto meta_opt = metadata_mgr.read(args.table.value());
        if (!meta_opt.has_value()) {
            std::cerr << "Error: Table not found: " << args.table.value() << std::endl;
            return 66;
        }
        Metadata meta = meta_opt.value();

        // Check for conflicts
        auto conflicts = ArgumentParser::detect_conflicts(args, meta);
        if (!conflicts.empty()) {
            std::cerr << "Error: Conflicts detected between CLI arguments and existing metadata:\n" << std::endl;
            for (const auto& conflict : conflicts) {
                std::cerr << conflict << "\n" << std::endl;
            }
            return 2;
        }

        // Initialize terminal
        Terminal terminal;
        if (!terminal.detect_size()) {
            std::cerr << "Warning: Could not detect terminal size, using defaults" << std::endl;
        }

        // Apply overrides if specified
        if (args.override_screen_height.has_value() && args.override_screen_width.has_value()) {
            int override_height = args.override_screen_height.value();
            int override_width = args.override_screen_width.value();

            // Validate that overrides don't exceed actual terminal size
            if (!terminal.validate_override_dimensions(override_height, override_width)) {
                std::cerr << "Error: Override dimensions (" << override_height << "x" << override_width
                          << ") exceed actual terminal size (" << terminal.actual_rows() << "x"
                          << terminal.actual_cols() << ")" << std::endl;
                return 64;
            }

            terminal.set_dimensions(override_height, override_width);
        }

        int screen_height = terminal.rows();
        int screen_width = terminal.cols();

        // Get valid ranges
        double x_min = meta.valid_x_min.value_or(-10.0);
        double x_max = meta.valid_x_max.value_or(10.0);
        double y_min = meta.valid_y_min.value_or(-10.0);
        double y_max = meta.valid_y_max.value_or(10.0);

        // Create viewport
        Viewport viewport(x_min, x_max, y_min, y_max,
                         x_min, x_max, y_min, y_max,  // Valid ranges
                         screen_height, screen_width);

        // Create data table
        DataTable data_table(db, args.table.value());

        // Create unsaved changes tracker
        UnsavedChanges unsaved_changes_tracker(db);

        // Calculate screen layout
        const int HEADER_ROWS = 3;
        const int FOOTER_ROWS = 1;
        const int edit_area_height = screen_height - HEADER_ROWS - FOOTER_ROWS;
        const int edit_area_start_row = HEADER_ROWS;

        // Calculate cursor position (centered)
        int cursor_row = edit_area_start_row + 1 + (edit_area_height - 2) / 2;
        int cursor_col = 1 + (screen_width - 2) / 2;

        // Clear buffer
        terminal.clear_buffer();

        // Query all data points
        auto all_points = data_table.query_viewport(
            viewport.data_x_min(), viewport.data_x_max(),
            viewport.data_y_min(), viewport.data_y_max()
        );

        // Count points
        int total_count = all_points.size();
        int x_count = 0;
        int o_count = 0;
        for (const auto& pt : all_points) {
            if (pt.target == meta.x_meaning) {
                x_count++;
            } else if (pt.target == meta.o_meaning) {
                o_count++;
            }
        }

        // Create renderers
        HeaderRenderer header_renderer;
        FooterRenderer footer_renderer;
        EditAreaRenderer edit_area_renderer;

        // Get current cursor position in data coordinates
        ScreenCoord cursor_content{cursor_row - edit_area_start_row - 1, cursor_col - 1};
        DataCoord cursor_data = viewport.screen_to_data(cursor_content);

        // Load unsaved changes for this table
        std::vector<ChangeRecord> unsaved_changes = unsaved_changes_tracker.get_changes(args.table.value());

        // Count active unsaved changes across all tables (for header display)
        auto all_changes = unsaved_changes_tracker.get_all_changes();
        int total_active_changes = 0;
        for (const auto& change : all_changes) {
            if (change.is_active) {
                total_active_changes++;
            }
        }

        // Count active unsaved changes for this table only (for footer display)
        int table_active_changes = 0;
        for (const auto& change : unsaved_changes) {
            if (change.is_active) {
                table_active_changes++;
            }
        }

        // Render header
        header_renderer.render(terminal, args.database.value(), meta.table_name,
                              meta.target_col_name, meta.x_meaning, meta.o_meaning,
                              total_count, x_count, o_count,
                              x_min, x_max, y_min, y_max,
                              viewport.data_x_min(), viewport.data_x_max(),
                              viewport.data_y_min(), viewport.data_y_max(), 0, total_active_changes);

        // Render edit area
        edit_area_renderer.render(terminal, viewport, data_table, unsaved_changes,
                                 edit_area_start_row, edit_area_height, screen_width,
                                 cursor_row, cursor_col, meta.x_meaning, meta.o_meaning);

        // Render footer
        footer_renderer.render(terminal, cursor_data.x, cursor_data.y,
                              x_min, x_max, y_min, y_max,
                              viewport.data_x_min(), viewport.data_x_max(),
                              viewport.data_y_min(), viewport.data_y_max(), 0, table_active_changes);

        // Output the buffer to stdout
        if (args.dump_screen) {
            // Dump entire screen
            for (int row = 0; row < screen_height; ++row) {
                std::cout << terminal.get_row(row);
                if (row < screen_height - 1) {
                    std::cout << '\n';
                }
            }
        } else {
            // Dump only edit area contents (inside the border)
            for (int row = edit_area_start_row + 1; row < edit_area_start_row + edit_area_height - 1; ++row) {
                // Get the full row
                std::string full_row = terminal.get_row(row);
                // Extract only the content inside the border (columns 1 to width-2)
                // Border is at column 0 (left) and column width-1 (right)
                // Content is in columns 1 through width-2
                if (full_row.length() > 1) {
                    int content_length = std::min(screen_width - 2, static_cast<int>(full_row.length()) - 1);
                    std::string content = full_row.substr(1, content_length);
                    std::cout << content;
                }
                if (row < edit_area_start_row + edit_area_height - 2) {
                    std::cout << '\n';
                }
            }
        }

        return 0;
    }

    // --key-stroke-at-point
    if (args.key_stroke_at_point.has_value()) {
        if (!args.table.has_value()) {
            std::cerr << "Error: --table is required for --key-stroke-at-point" << std::endl;
            return 2;
        }

        // Parse x,y,key from the string
        std::string input = args.key_stroke_at_point.value();
        size_t first_comma = input.find(',');
        size_t second_comma = input.find(',', first_comma + 1);

        if (first_comma == std::string::npos || second_comma == std::string::npos) {
            std::cerr << "Error: --key-stroke-at-point requires format x,y,key (e.g. 1.5,2.3,x)" << std::endl;
            return 2;
        }

        std::string x_str = input.substr(0, first_comma);
        std::string y_str = input.substr(first_comma + 1, second_comma - first_comma - 1);
        std::string key_str = input.substr(second_comma + 1);

        // Parse x and y coordinates
        double point_x, point_y;
        try {
            point_x = std::stod(x_str);
            point_y = std::stod(y_str);
        } catch (const std::exception& e) {
            std::cerr << "Error: Invalid x,y coordinates in --key-stroke-at-point: " << e.what() << std::endl;
            return 2;
        }

        // Validate key is a single character
        if (key_str.length() != 1) {
            std::cerr << "Error: --key-stroke-at-point key must be a single character" << std::endl;
            return 2;
        }
        char key = key_str[0];

        // Validate key is one of the supported keys
        if (key != 'x' && key != 'o' && key != ' ' && key != 'X' && key != 'O' && key != 'g') {
            std::cerr << "Error: --key-stroke-at-point key must be one of: x, o, space, X, O, g" << std::endl;
            return 2;
        }

        // Load metadata
        MetadataManager metadata_mgr(db);
        auto meta_opt = metadata_mgr.read(args.table.value());
        if (!meta_opt.has_value()) {
            std::cerr << "Error: Table not found: " << args.table.value() << std::endl;
            return 66;
        }
        Metadata meta = meta_opt.value();

        // Check for conflicts
        auto conflicts = ArgumentParser::detect_conflicts(args, meta);
        if (!conflicts.empty()) {
            std::cerr << "Error: Conflicts detected between CLI arguments and existing metadata:\n" << std::endl;
            for (const auto& conflict : conflicts) {
                std::cerr << conflict << "\n" << std::endl;
            }
            return 2;
        }

        // Initialize terminal
        Terminal terminal;
        if (!terminal.detect_size()) {
            std::cerr << "Warning: Could not detect terminal size, using defaults" << std::endl;
        }

        // Apply overrides if specified
        if (args.override_screen_height.has_value() && args.override_screen_width.has_value()) {
            int override_height = args.override_screen_height.value();
            int override_width = args.override_screen_width.value();

            // Validate that overrides don't exceed actual terminal size
            if (!terminal.validate_override_dimensions(override_height, override_width)) {
                std::cerr << "Error: Override dimensions (" << override_height << "x" << override_width
                          << ") exceed actual terminal size (" << terminal.actual_rows() << "x"
                          << terminal.actual_cols() << ")" << std::endl;
                return 64;
            }

            terminal.set_dimensions(override_height, override_width);
        }

        int screen_height = terminal.rows();
        int screen_width = terminal.cols();

        // Get valid ranges
        double x_min = meta.valid_x_min.value_or(-10.0);
        double x_max = meta.valid_x_max.value_or(10.0);
        double y_min = meta.valid_y_min.value_or(-10.0);
        double y_max = meta.valid_y_max.value_or(10.0);

        // Create viewport
        Viewport viewport(x_min, x_max, y_min, y_max,
                         x_min, x_max, y_min, y_max,  // Valid ranges
                         screen_height, screen_width);

        // Create data table
        DataTable data_table(db, args.table.value());

        // Create unsaved changes tracker
        UnsavedChanges unsaved_changes_tracker(db);

        // Calculate screen layout
        const int HEADER_ROWS = 3;
        const int FOOTER_ROWS = 1;
        const int edit_area_height = screen_height - HEADER_ROWS - FOOTER_ROWS;

        // Create point editor
        PointEditor editor(db, args.table.value());

        // Calculate cell size for hit testing
        // Use the viewport's data-per-pixel ratio
        double data_width = viewport.data_x_max() - viewport.data_x_min();
        double data_height = viewport.data_y_max() - viewport.data_y_min();
        double cell_width = data_width / (screen_width - 2);  // Subtract 2 for borders
        double cell_height = data_height / (edit_area_height - 2);
        double cell_size = std::max(cell_width, cell_height);

        // Simulate the keystroke
        int affected_count = 0;
        switch (key) {
            case 'x':
            case 'o':
                affected_count = editor.create_point(point_x, point_y, key) ? 1 : 0;
                break;
            case ' ':
                affected_count = editor.delete_points_at_cursor(point_x, point_y, cell_size);
                break;
            case 'X':
                affected_count = editor.convert_points_at_cursor(point_x, point_y, cell_size, 'x');
                break;
            case 'O':
                affected_count = editor.convert_points_at_cursor(point_x, point_y, cell_size, 'o');
                break;
            case 'g':
                affected_count = editor.flip_points_at_cursor(point_x, point_y, cell_size);
                break;
        }

        // Output the result (number of affected points)
        std::cout << affected_count << std::endl;

        return 0;
    }

    // If we got here with a database but no recognized command, show TUI table selection menu
    if (!args.table.has_value()) {
        // Initialize terminal for TUI menu
        Terminal menu_terminal;
        if (!menu_terminal.detect_size()) {
            std::cerr << "Warning: Could not detect terminal size, using defaults" << std::endl;
        }

        // Enter raw mode for TUI
        if (!menu_terminal.enter_raw_mode()) {
            std::cerr << "Error: Could not enter raw terminal mode" << std::endl;
            return 1;
        }

        // List available tables
        TableManager table_mgr(db);
        auto tables = table_mgr.list_tables();

        // Run the interactive TUI menu
        TableSelectionMenu menu(menu_terminal);
        MenuResult result = menu.run(tables);

        // Exit raw mode before handling results
        menu_terminal.exit_raw_mode();

        // Clear screen
        std::cout << "\033[2J\033[H";

        // Handle the menu result
        if (result.action == MenuAction::EXIT) {
            return 0;
        } else if (result.action == MenuAction::OPEN_TABLE) {
            // Open the selected table
            if (result.table_name.has_value()) {
                args.table = result.table_name.value();
            } else {
                std::cerr << "Error: No table selected" << std::endl;
                return 2;
            }
        } else if (result.action == MenuAction::CREATE_TABLE) {
            // Prompt for table creation details
            std::string table_name, target_col, x_axis, y_axis, x_meaning, o_meaning;
            double min_x = -10.0, max_x = 10.0, min_y = -10.0, max_y = 10.0;

            std::cout << "Create New Table\n" << std::endl;
            std::cout << "Table name: ";
            std::getline(std::cin, table_name);
            std::cout << "Target column name (e.g., 'label', 'class'): ";
            std::getline(std::cin, target_col);
            std::cout << "X-axis name (e.g., 'x', 'feature1'): ";
            std::getline(std::cin, x_axis);
            std::cout << "Y-axis name (e.g., 'y', 'feature2'): ";
            std::getline(std::cin, y_axis);
            std::cout << "X meaning (label for 'x' points): ";
            std::getline(std::cin, x_meaning);
            std::cout << "O meaning (label for 'o' points): ";
            std::getline(std::cin, o_meaning);

            std::cout << "Min X [" << min_x << "]: ";
            std::string input;
            std::getline(std::cin, input);
            if (!input.empty()) min_x = std::stod(input);

            std::cout << "Max X [" << max_x << "]: ";
            std::getline(std::cin, input);
            if (!input.empty()) max_x = std::stod(input);

            std::cout << "Min Y [" << min_y << "]: ";
            std::getline(std::cin, input);
            if (!input.empty()) min_y = std::stod(input);

            std::cout << "Max Y [" << max_y << "]: ";
            std::getline(std::cin, input);
            if (!input.empty()) max_y = std::stod(input);

            // Create the table
            if (table_mgr.create_table(table_name, target_col, x_axis, y_axis,
                                      x_meaning, o_meaning, min_x, max_x, min_y, max_y, false)) {
                std::cout << "\nTable '" << table_name << "' created successfully!" << std::endl;
                std::cout << "Opening table in interactive mode...\n" << std::endl;
                args.table = table_name;
            } else {
                std::cerr << "Error: Failed to create table" << std::endl;
                return 66;
            }
        } else if (result.action == MenuAction::DELETE_TABLE) {
            // Prompt for table name to delete
            std::string table_name;
            std::cout << "Enter table name to delete: ";
            std::getline(std::cin, table_name);

            if (table_mgr.delete_table(table_name)) {
                std::cout << "Table '" << table_name << "' deleted successfully." << std::endl;
            } else {
                std::cerr << "Error: Failed to delete table" << std::endl;
            }
            return 0;
        } else if (result.action == MenuAction::VIEW_METADATA) {
            // Prompt for table name to view
            std::string table_name;
            std::cout << "Enter table name: ";
            std::getline(std::cin, table_name);

            if (!table_mgr.show_metadata(table_name, std::cout)) {
                std::cerr << "Error: Table not found" << std::endl;
            }
            return 0;
        }

        // If we don't have a table yet, exit
        if (!args.table.has_value()) {
            return 0;
        }
    }

    // Start interactive TUI mode
    std::string table_name = args.table.value();

    // Load metadata
    MetadataManager metadata_mgr(db);
    auto meta_opt = metadata_mgr.read(table_name);
    if (!meta_opt.has_value()) {
        // Table not found - show available tables
        TableManager table_mgr(db);
        auto tables = table_mgr.list_tables();

        std::cerr << "Error: Table not found: " << table_name << std::endl;
        if (!tables.empty()) {
            std::cerr << "\nAvailable tables:" << std::endl;
            for (const auto& table : tables) {
                std::cerr << "  " << table << std::endl;
            }
        } else {
            std::cerr << "\nNo tables exist in this database." << std::endl;
            std::cerr << "Use --create-table to create one." << std::endl;
        }
        return 66;
    }
    Metadata meta = meta_opt.value();

    // Check for conflicts between CLI arguments and existing metadata
    auto conflicts = ArgumentParser::detect_conflicts(args, meta);
    if (!conflicts.empty()) {
        std::cerr << "Error: Conflicts detected between CLI arguments and existing metadata:\n" << std::endl;
        for (const auto& conflict : conflicts) {
            std::cerr << conflict << "\n" << std::endl;
        }
        return 2;
    }

    // Initialize terminal
    Terminal terminal;
    if (!terminal.detect_size()) {
        std::cerr << "Warning: Could not detect terminal size, using defaults" << std::endl;
    }
    if (!terminal.is_size_adequate()) {
        std::cerr << "Error: Terminal too small (need at least 5 rows x 40 cols)" << std::endl;
        return 1;
    }

    // Apply overrides if specified
    if (args.override_screen_height.has_value() && args.override_screen_width.has_value()) {
        int override_height = args.override_screen_height.value();
        int override_width = args.override_screen_width.value();

        // Validate that overrides don't exceed actual terminal size
        if (!terminal.validate_override_dimensions(override_height, override_width)) {
            std::cerr << "Error: Override dimensions (" << override_height << "x" << override_width
                      << ") exceed actual terminal size (" << terminal.actual_rows() << "x" << terminal.actual_cols() << ")"
                      << std::endl;
            return 64;
        }

        terminal.set_dimensions(override_height, override_width);
    }

    // Initialize viewport
    int screen_height = terminal.rows();
    int screen_width = terminal.cols();

    // Unwrap optional values (they should exist for valid metadata)
    double x_min = meta.valid_x_min.value_or(-10.0);
    double x_max = meta.valid_x_max.value_or(10.0);
    double y_min = meta.valid_y_min.value_or(-10.0);
    double y_max = meta.valid_y_max.value_or(10.0);

    // Create viewport with valid ranges specified
    // Initial viewport shows entire valid range
    Viewport viewport(x_min, x_max, y_min, y_max,
                     x_min, x_max, y_min, y_max,  // Valid ranges
                     screen_height, screen_width);

    // Create data table
    DataTable data_table(db, table_name);

    // Create point editor for handling x/o creation
    PointEditor point_editor(db, table_name);

    // Create unsaved changes tracker
    UnsavedChanges unsaved_changes_tracker(db);

    // Enter raw mode
    if (!terminal.enter_raw_mode()) {
        std::cerr << "Error: Could not enter raw terminal mode" << std::endl;
        return 1;
    }

    // Calculate screen layout
    const int HEADER_ROWS = 3;  // Header takes 3 rows
    const int FOOTER_ROWS = 1;  // Footer takes 1 row
    const int edit_area_height = screen_height - HEADER_ROWS - FOOTER_ROWS;
    const int edit_area_start_row = HEADER_ROWS;

    // Helper lambda to convert cursor position from screen coordinates to edit area content coordinates
    auto cursor_to_content_coords = [&](int cursor_screen_row, int cursor_screen_col) -> ScreenCoord {
        // Convert from absolute screen coordinates to edit area content coordinates (0-based)
        // Subtract edit area offset and border (1 char)
        int content_row = cursor_screen_row - edit_area_start_row - 1;
        int content_col = cursor_screen_col - 1;
        return {content_row, content_col};
    };

    // Main TUI loop
    bool running = true;
    bool needs_redraw = true;
    // Cursor is within edit area content (inside border)
    // Border takes 1 row at top/bottom and 1 col at left/right
    int cursor_row = edit_area_start_row + 1 + (edit_area_height - 2) / 2;
    int cursor_col = 1 + (screen_width - 2) / 2;

    std::cout << "Starting DataPainter TUI..." << std::endl;
    std::cout << "Keys: q=quit, +/-=zoom, arrows=move, x/o=add point, backspace=delete" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));

    while (running) {
        if (needs_redraw) {
            // Clear buffer
            terminal.clear_buffer();

            // Query all data points
            auto all_points = data_table.query_viewport(
                viewport.data_x_min(), viewport.data_x_max(),
                viewport.data_y_min(), viewport.data_y_max()
            );

            // Count points
            int total_count = all_points.size();
            int x_count = 0;
            int o_count = 0;
            for (const auto& pt : all_points) {
                if (pt.target == meta.x_meaning) {
                    x_count++;
                } else if (pt.target == meta.o_meaning) {
                    o_count++;
                }
            }

            // Create renderers
            HeaderRenderer header_renderer;
            FooterRenderer footer_renderer;
            EditAreaRenderer edit_area_renderer;

            // Get current cursor position in data coordinates
            ScreenCoord cursor_content = cursor_to_content_coords(cursor_row, cursor_col);
            DataCoord cursor_data = viewport.screen_to_data(cursor_content);

            // Load unsaved changes for this table
            std::vector<ChangeRecord> unsaved_changes = unsaved_changes_tracker.get_changes(table_name);

            // Count active unsaved changes across all tables (for header display)
            auto all_changes = unsaved_changes_tracker.get_all_changes();
            int total_active_changes = 0;
            for (const auto& change : all_changes) {
                if (change.is_active) {
                    total_active_changes++;
                }
            }

            // Count active unsaved changes for this table only (for footer display)
            int table_active_changes = 0;
            for (const auto& change : unsaved_changes) {
                if (change.is_active) {
                    table_active_changes++;
                }
            }

            // Render header
            header_renderer.render(terminal, args.database.value(), meta.table_name,
                                  meta.target_col_name, meta.x_meaning, meta.o_meaning,
                                  total_count, x_count, o_count,
                                  x_min, x_max, y_min, y_max,
                                  viewport.data_x_min(), viewport.data_x_max(),
                                  viewport.data_y_min(), viewport.data_y_max(), 0, total_active_changes);

            // Render edit area
            edit_area_renderer.render(terminal, viewport, data_table, unsaved_changes,
                                     edit_area_start_row, edit_area_height, screen_width,
                                     cursor_row, cursor_col, meta.x_meaning, meta.o_meaning);

            // Render footer
            footer_renderer.render(terminal, cursor_data.x, cursor_data.y,
                                  x_min, x_max, y_min, y_max,
                                  viewport.data_x_min(), viewport.data_x_max(),
                                  viewport.data_y_min(), viewport.data_y_max(), 0, table_active_changes);

            // Display to screen with cursor
            terminal.render_with_cursor(cursor_row, cursor_col);
            needs_redraw = false;
        }

        // Read keyboard input
        int key = terminal.read_key();
        if (key >= 0) {
            // Handle arrow keys (from ncurses or our own codes)
            if (key == Terminal::KEY_UP_ARROW) {
                // Up arrow - move cursor up (within edit area content, inside border)
                // Border is at edit_area_start_row, content starts at edit_area_start_row + 1
                if (cursor_row > edit_area_start_row + 1) {
                    // Check if new position would be within valid ranges
                    int new_cursor_row = cursor_row - 1;
                    if (is_cursor_position_valid(viewport, new_cursor_row, cursor_col, edit_area_start_row)) {
                        cursor_row = new_cursor_row;
                        needs_redraw = true;
                    }
                } else if (cursor_row == edit_area_start_row + 1) {
                    // Cursor is at top edge, try to pan up
                    // Pan up shifts viewport up (increases y_min and y_max)
                    double old_y_max = viewport.data_y_max();
                    viewport.pan_up();
                    // If viewport actually panned, redraw
                    if (viewport.data_y_max() != old_y_max) {
                        needs_redraw = true;
                    }
                }
            }
            else if (key == Terminal::KEY_DOWN_ARROW) {
                // Down arrow - move cursor down (within edit area content, inside border)
                // Border is at edit_area_start_row + edit_area_height - 1
                // Content ends at edit_area_start_row + edit_area_height - 2
                int edit_area_end_row = edit_area_start_row + edit_area_height - 2;
                if (cursor_row < edit_area_end_row) {
                    // Check if new position would be within valid ranges
                    int new_cursor_row = cursor_row + 1;
                    if (is_cursor_position_valid(viewport, new_cursor_row, cursor_col, edit_area_start_row)) {
                        cursor_row = new_cursor_row;
                        needs_redraw = true;
                    }
                } else if (cursor_row == edit_area_end_row) {
                    // Cursor is at bottom edge, try to pan down
                    // Pan down shifts viewport down (decreases y_min and y_max)
                    double old_y_min = viewport.data_y_min();
                    viewport.pan_down();
                    // If viewport actually panned, redraw
                    if (viewport.data_y_min() != old_y_min) {
                        needs_redraw = true;
                    }
                }
            }
            else if (key == Terminal::KEY_LEFT_ARROW) {
                // Left arrow - move cursor left (inside border at column 1)
                if (cursor_col > 1) {
                    // Check if new position would be within valid ranges
                    int new_cursor_col = cursor_col - 1;
                    if (is_cursor_position_valid(viewport, cursor_row, new_cursor_col, edit_area_start_row)) {
                        cursor_col = new_cursor_col;
                        needs_redraw = true;
                    }
                } else if (cursor_col == 1) {
                    // Cursor is at left edge, try to pan left
                    // Pan left shifts viewport left (decreases x_min and x_max)
                    double old_x_min = viewport.data_x_min();
                    viewport.pan_left();
                    // If viewport actually panned, redraw
                    if (viewport.data_x_min() != old_x_min) {
                        needs_redraw = true;
                    }
                }
            }
            else if (key == Terminal::KEY_RIGHT_ARROW) {
                // Right arrow - move cursor right (inside border at column screen_width - 2)
                if (cursor_col < screen_width - 2) {
                    // Check if new position would be within valid ranges
                    int new_cursor_col = cursor_col + 1;
                    if (is_cursor_position_valid(viewport, cursor_row, new_cursor_col, edit_area_start_row)) {
                        cursor_col = new_cursor_col;
                        needs_redraw = true;
                    }
                } else if (cursor_col == screen_width - 2) {
                    // Cursor is at right edge, try to pan right
                    // Pan right shifts viewport right (increases x_min and x_max)
                    double old_x_max = viewport.data_x_max();
                    viewport.pan_right();
                    // If viewport actually panned, redraw
                    if (viewport.data_x_max() != old_x_max) {
                        needs_redraw = true;
                    }
                }
            }
            // Handle quit (q, Q, or ESC)
            else if (key == 'q' || key == 'Q' || key == 27) {
                // Check for unsaved changes
                auto all_changes = unsaved_changes_tracker.get_all_changes();
                int active_changes = 0;
                for (const auto& change : all_changes) {
                    if (change.is_active) {
                        active_changes++;
                    }
                }

                if (active_changes == 0) {
                    // No unsaved changes, quit immediately
                    running = false;
                } else {
                    // Unsaved changes exist, show confirmation dialog
                    // Exit raw mode temporarily to show dialog
                    terminal.exit_raw_mode();

                    // Clear screen and show dialog
                    std::cout << "\033[2J\033[H";  // Clear screen
                    std::cout << "You have " << active_changes << " unsaved change";
                    if (active_changes != 1) std::cout << "s";
                    std::cout << "." << std::endl;
                    std::cout << std::endl;
                    std::cout << "Save changes before quitting?" << std::endl;
                    std::cout << "  y - Save and quit" << std::endl;
                    std::cout << "  n - Discard changes and quit" << std::endl;
                    std::cout << "  c - Cancel (return to editor)" << std::endl;
                    std::cout << std::endl;
                    std::cout << "Your choice: ";
                    std::cout.flush();

                    // Read user's choice
                    char choice;
                    std::cin >> choice;
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                    if (choice == 'y' || choice == 'Y') {
                        // Save then quit
                        SaveManager save_manager(db, table_name);
                        bool save_success = save_manager.save();
                        if (save_success) {
                            running = false;
                        } else {
                            // Show error message
                            std::cout << "\nError: Failed to save changes. Press Enter to return to editor.";
                            std::cin.get();  // Wait for Enter
                            // Re-enter raw mode and continue
                            terminal.enter_raw_mode();
                            needs_redraw = true;
                        }
                    } else if (choice == 'n' || choice == 'N') {
                        // Discard and quit - clear all unsaved changes
                        unsaved_changes_tracker.clear_all_changes();
                        running = false;
                    } else {
                        // Cancel or any other input - return to editor
                        terminal.enter_raw_mode();
                        needs_redraw = true;
                    }
                }
            }
            // Handle zoom
            else if (key == '+' || key == '=') {
                // Get cursor's current data coordinates
                ScreenCoord cursor_content = cursor_to_content_coords(cursor_row, cursor_col);
                DataCoord cursor_data = viewport.screen_to_data(cursor_content);

                // Zoom in (with smart centering to maximize usable area)
                viewport.zoom_in(cursor_data);

                // Update cursor screen position to maintain same data coordinates
                auto new_screen_pos = viewport.data_to_screen(cursor_data);
                if (new_screen_pos.has_value()) {
                    cursor_row = new_screen_pos->row;
                    cursor_col = new_screen_pos->col;

                    // Ensure cursor stays within content area bounds (inside border)
                    cursor_row = std::max(edit_area_start_row + 1,
                                         std::min(cursor_row, edit_area_start_row + edit_area_height - 2));
                    cursor_col = std::max(1, std::min(cursor_col, screen_width - 2));
                }

                needs_redraw = true;
            }
            else if (key == '-' || key == '_') {
                // Get cursor's current data coordinates
                ScreenCoord cursor_content = cursor_to_content_coords(cursor_row, cursor_col);
                DataCoord cursor_data = viewport.screen_to_data(cursor_content);

                // Zoom out
                viewport.zoom_out(cursor_data);

                // Update cursor screen position to maintain same data coordinates
                auto new_screen_pos = viewport.data_to_screen(cursor_data);
                if (new_screen_pos.has_value()) {
                    cursor_row = new_screen_pos->row;
                    cursor_col = new_screen_pos->col;

                    // Ensure cursor stays within content area bounds (inside border)
                    cursor_row = std::max(edit_area_start_row + 1,
                                         std::min(cursor_row, edit_area_start_row + edit_area_height - 2));
                    cursor_col = std::max(1, std::min(cursor_col, screen_width - 2));
                }

                needs_redraw = true;
            }
            // Handle point creation and editing
            else if (key == 'x' || key == 'o') {
                // Create a point at cursor position
                ScreenCoord cursor_content = cursor_to_content_coords(cursor_row, cursor_col);
                DataCoord cursor_data = viewport.screen_to_data(cursor_content);

                // Create point (PointEditor will record in unsaved_changes)
                if (point_editor.create_point(cursor_data.x, cursor_data.y, static_cast<char>(key))) {
                    needs_redraw = true;
                }
            }
            else if (key == 'X') {
                // Convert all 'o' points at cursor to 'x'
                ScreenCoord cursor_content = cursor_to_content_coords(cursor_row, cursor_col);
                DataCoord cursor_data = viewport.screen_to_data(cursor_content);
                double cell_size_x = (viewport.data_x_max() - viewport.data_x_min()) / (screen_width - 2);
                double cell_size = cell_size_x;  // Use x cell size for hit testing

                int converted = point_editor.convert_points_at_cursor(cursor_data.x, cursor_data.y, cell_size, 'x');
                if (converted > 0) {
                    needs_redraw = true;
                }
            }
            else if (key == 'O') {
                // Convert all 'x' points at cursor to 'o'
                ScreenCoord cursor_content = cursor_to_content_coords(cursor_row, cursor_col);
                DataCoord cursor_data = viewport.screen_to_data(cursor_content);
                double cell_size_x = (viewport.data_x_max() - viewport.data_x_min()) / (screen_width - 2);
                double cell_size = cell_size_x;

                int converted = point_editor.convert_points_at_cursor(cursor_data.x, cursor_data.y, cell_size, 'o');
                if (converted > 0) {
                    needs_redraw = true;
                }
            }
            else if (key == 'g') {
                // Flip all points at cursor (x ↔ o)
                ScreenCoord cursor_content = cursor_to_content_coords(cursor_row, cursor_col);
                DataCoord cursor_data = viewport.screen_to_data(cursor_content);
                double cell_size_x = (viewport.data_x_max() - viewport.data_x_min()) / (screen_width - 2);
                double cell_size = cell_size_x;

                int flipped = point_editor.flip_points_at_cursor(cursor_data.x, cursor_data.y, cell_size);
                if (flipped > 0) {
                    needs_redraw = true;
                }
            }
            else if (key == '?') {
                // Show help overlay
                HelpOverlay help;
                terminal.clear_buffer();
                help.render(terminal, screen_height, screen_width);
                terminal.render_with_cursor(cursor_row, cursor_col);

                // Wait for any key press to dismiss
                terminal.read_key();

                // Redraw the main UI after dismissing help
                needs_redraw = true;
            }
            else if (key == 's' || key == 'S') {
                // Save unsaved changes to database
                SaveManager save_manager(db, table_name);
                bool save_success = save_manager.save();

                if (save_success) {
                    // Save successful, redraw to update unsaved count
                    needs_redraw = true;
                } else {
                    // Save failed - show error message
                    terminal.exit_raw_mode();
                    std::cerr << "Error: Failed to save changes to database" << std::endl;
                    std::cerr << "Press Enter to continue..." << std::endl;
                    std::cin.get();
                    terminal.enter_raw_mode();
                    needs_redraw = true;
                }
            }
            else if (key == 127 || key == 8) {
                // Delete all points at cursor (backspace or delete key)
                ScreenCoord cursor_content = cursor_to_content_coords(cursor_row, cursor_col);
                DataCoord cursor_data = viewport.screen_to_data(cursor_content);
                double cell_size_x = (viewport.data_x_max() - viewport.data_x_min()) / (screen_width - 2);
                double cell_size = cell_size_x;

                int deleted = point_editor.delete_points_at_cursor(cursor_data.x, cursor_data.y, cell_size);
                if (deleted > 0) {
                    needs_redraw = true;
                }
            }
        }

        // Small delay to prevent busy-waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    // Exit raw mode
    terminal.exit_raw_mode();

    // Clear screen and show exit message
    std::cout << "\033[2J\033[H";  // Clear screen
    std::cout << "DataPainter exited successfully." << std::endl;

    return 0;
}
