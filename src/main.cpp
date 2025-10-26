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
#include <algorithm>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

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
        // No database operations requested, just show version
        std::cout << "DataPainter v0.1.0" << std::endl;
        std::cout << "TUI not yet implemented. See TODO.md for development roadmap." << std::endl;
        std::cout << "Run with --list-tables, --create-table, etc. for non-interactive operations." << std::endl;
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

    // If we got here with a database but no recognized command, start interactive mode
    if (!args.table.has_value()) {
        std::cout << "DataPainter v0.1.0" << std::endl;
        std::cout << "Interactive mode requires --table argument." << std::endl;
        std::cout << "Use --list-tables to see available tables." << std::endl;
        return 0;
    }

    // Start interactive TUI mode
    std::string table_name = args.table.value();

    // Load metadata
    MetadataManager metadata_mgr(db);
    auto meta_opt = metadata_mgr.read(table_name);
    if (!meta_opt.has_value()) {
        std::cerr << "Error: Table not found: " << table_name << std::endl;
        return 66;
    }
    Metadata meta = meta_opt.value();

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
        terminal.set_dimensions(args.override_screen_height.value(),
                               args.override_screen_width.value());
    }

    // Initialize viewport
    int screen_height = terminal.rows();
    int screen_width = terminal.cols();

    // Unwrap optional values (they should exist for valid metadata)
    double x_min = meta.valid_x_min.value_or(-10.0);
    double x_max = meta.valid_x_max.value_or(10.0);
    double y_min = meta.valid_y_min.value_or(-10.0);
    double y_max = meta.valid_y_max.value_or(10.0);

    Viewport viewport(x_min, x_max, y_min, y_max,
                     screen_height, screen_width);

    // Create data table
    DataTable data_table(db, table_name);

    // Enter raw mode
    if (!terminal.enter_raw_mode()) {
        std::cerr << "Error: Could not enter raw terminal mode" << std::endl;
        return 1;
    }

    // Main TUI loop
    bool running = true;
    bool needs_redraw = true;
    int cursor_row = screen_height / 2;
    int cursor_col = screen_width / 2;

    std::cout << "Starting DataPainter TUI..." << std::endl;
    std::cout << "Press 'q' to quit, '+'/'-' to zoom, WASD to pan" << std::endl;
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
            DataCoord cursor_data = viewport.screen_to_data({cursor_row, cursor_col});

            // Render header
            header_renderer.render(terminal, args.database.value(), meta.table_name,
                                  meta.target_col_name, meta.x_meaning, meta.o_meaning,
                                  total_count, x_count, o_count,
                                  x_min, x_max, y_min, y_max, 0);

            // Render edit area
            std::vector<ChangeRecord> unsaved_changes;  // Empty for now
            edit_area_renderer.render(terminal, viewport, data_table, unsaved_changes,
                                     cursor_row, cursor_col, meta.x_meaning, meta.o_meaning);

            // Render footer
            footer_renderer.render(terminal, cursor_data.x, cursor_data.y,
                                  x_min, x_max, y_min, y_max, 0);

            // Display to screen
            terminal.render();
            needs_redraw = false;
        }

        // Read keyboard input
        int key = terminal.read_key();
        if (key >= 0) {
            char ch = static_cast<char>(key);

            // Handle quit
            if (ch == 'q' || ch == 'Q' || ch == 27) {  // 'q' or ESC
                running = false;
            }
            // Handle zoom
            else if (ch == '+' || ch == '=') {
                DataCoord center = viewport.screen_to_data({cursor_row, cursor_col});
                viewport.zoom_in(center);
                needs_redraw = true;
            }
            else if (ch == '-' || ch == '_') {
                DataCoord center = viewport.screen_to_data({cursor_row, cursor_col});
                viewport.zoom_out(center);
                needs_redraw = true;
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
