#pragma once

#include <string>
#include <vector>
#include <optional>
#include "terminal.h"

namespace datapainter {

// Represents a user action from the table selection menu
enum class MenuAction {
    OPEN_TABLE,     // Open an existing table
    CREATE_TABLE,   // Create a new table
    DELETE_TABLE,   // Delete an existing table
    VIEW_METADATA,  // View table metadata
    EXIT            // Exit the application
};

// Result of running the menu
struct MenuResult {
    MenuAction action;
    std::optional<std::string> table_name;  // Selected table (if applicable)
};

// TUI menu for table selection and management
class TableSelectionMenu {
public:
    explicit TableSelectionMenu(Terminal& terminal);

    // Render the menu with a list of tables and current selection
    // selected_index: the currently highlighted item (0-based)
    void render(const std::vector<std::string>& tables, int selected_index);

    // Normalize selection index to valid range (handles wrapping)
    // Returns a valid index within [0, max_items)
    int normalize_selection(int index, const std::vector<std::string>& tables) const;

    // Run the interactive menu loop
    // Returns the user's selected action and table name (if applicable)
    // Returns MenuAction::EXIT if user cancels
    MenuResult run(const std::vector<std::string>& tables);

private:
    Terminal& terminal_;

    // Calculate the number of selectable items (tables + actions)
    int get_item_count(const std::vector<std::string>& tables) const;

    // Draw the border around the menu
    void draw_border(int start_row, int height, int width);

    // Render table list
    void render_table_list(const std::vector<std::string>& tables, int selected_index,
                           int start_row, int max_height);

    // Render action menu
    void render_actions(const std::vector<std::string>& tables, int selected_index, int start_row);

    // Convert selection index to MenuAction
    MenuAction index_to_action(int index, const std::vector<std::string>& tables) const;

    // Get the selected table name from index (if applicable)
    std::optional<std::string> index_to_table(int index,
                                              const std::vector<std::string>& tables) const;
};

}  // namespace datapainter
