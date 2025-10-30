#include "table_selection_menu.h"
#include <algorithm>
#include <sstream>

namespace datapainter {

TableSelectionMenu::TableSelectionMenu(Terminal& terminal) : terminal_(terminal) {}

void TableSelectionMenu::render(const std::vector<std::string>& tables, int selected_index) {
    terminal_.clear_buffer();

    int screen_height = terminal_.rows();
    int screen_width = terminal_.cols();

    // Check if terminal is too small
    if (!terminal_.is_size_adequate()) {
        // Show "enlarge terminal" message
        std::string msg = "Please enlarge your terminal";
        std::string size_msg = "(minimum: 5 rows x 40 columns)";

        int msg_row = screen_height / 2;
        int msg_col = (screen_width - msg.length()) / 2;
        if (msg_col < 0) msg_col = 0;

        for (size_t i = 0; i < msg.length() && msg_col + i < static_cast<size_t>(screen_width); ++i) {
            terminal_.write_char(msg_row, msg_col + i, msg[i]);
        }

        if (msg_row + 1 < screen_height) {
            int size_col = (screen_width - size_msg.length()) / 2;
            if (size_col < 0) size_col = 0;
            for (size_t i = 0; i < size_msg.length() && size_col + i < static_cast<size_t>(screen_width); ++i) {
                terminal_.write_char(msg_row + 1, size_col + i, size_msg[i]);
            }
        }
        return;  // Don't render normal menu
    }

    // Title at top with terminal dimensions
    std::string title = "DataPainter - Table Selection";
    std::string size_info = " [" + std::to_string(screen_height) + "x" + std::to_string(screen_width) + "]";
    std::string full_title = title + size_info;

    int title_col = (screen_width - full_title.length()) / 2;
    if (title_col < 0) title_col = 0;

    for (size_t i = 0; i < full_title.length() && title_col + i < static_cast<size_t>(screen_width); ++i) {
        terminal_.write_char(0, title_col + i, full_title[i]);
    }

    // Draw border for menu area (starting at row 2)
    int menu_start_row = 2;
    int menu_height = screen_height - 3;  // Leave room for title and padding
    draw_border(menu_start_row, menu_height, screen_width);

    // Render table list if tables exist
    if (!tables.empty()) {
        render_table_list(tables, selected_index, menu_start_row + 2, menu_height - 4);
    } else {
        // Show "No tables" message
        std::string msg = "No tables found in database";
        int msg_row = menu_start_row + 3;
        int msg_col = (screen_width - msg.length()) / 2;
        for (size_t i = 0; i < msg.length(); ++i) {
            terminal_.write_char(msg_row, msg_col + i, msg[i]);
        }
    }

    // Render action menu at bottom
    render_actions(tables, selected_index, screen_height - 10);
}

void TableSelectionMenu::draw_border(int start_row, int height, int width) {
    int end_row = start_row + height - 1;
    int end_col = width - 1;

    // Draw corners
    terminal_.write_acs(start_row, 0, Terminal::AcsChar::ULCORNER);
    terminal_.write_acs(start_row, end_col, Terminal::AcsChar::URCORNER);
    terminal_.write_acs(end_row, 0, Terminal::AcsChar::LLCORNER);
    terminal_.write_acs(end_row, end_col, Terminal::AcsChar::LRCORNER);

    // Draw horizontal edges
    for (int col = 1; col < end_col; ++col) {
        terminal_.write_acs(start_row, col, Terminal::AcsChar::HLINE);
        terminal_.write_acs(end_row, col, Terminal::AcsChar::HLINE);
    }

    // Draw vertical edges
    for (int row = start_row + 1; row < end_row; ++row) {
        terminal_.write_acs(row, 0, Terminal::AcsChar::VLINE);
        terminal_.write_acs(row, end_col, Terminal::AcsChar::VLINE);
    }
}

void TableSelectionMenu::render_table_list(const std::vector<std::string>& tables,
                                           int selected_index, int start_row, int max_height) {
    // Header
    std::string header = "Available Tables:";
    for (size_t i = 0; i < header.length(); ++i) {
        terminal_.write_char(start_row, 4, header[i]);
    }

    // List tables
    int row = start_row + 2;
    for (size_t i = 0; i < tables.size() && row < start_row + max_height; ++i) {
        bool is_selected = (static_cast<int>(i) == selected_index && selected_index < static_cast<int>(tables.size()));

        // Selection indicator
        char indicator = is_selected ? '>' : ' ';
        terminal_.write_char(row, 6, indicator);

        // Table name
        std::string display = std::to_string(i + 1) + ". " + tables[i];
        for (size_t j = 0; j < display.length(); ++j) {
            terminal_.write_char(row, 8 + j, display[j]);
        }

        row++;
    }
}

void TableSelectionMenu::render_actions(const std::vector<std::string>& tables,
                                       int selected_index, int start_row) {
    std::string header = "Actions:";
    for (size_t i = 0; i < header.length(); ++i) {
        terminal_.write_char(start_row, 4 + i, header[i]);
    }

    int row = start_row + 2;
    int action_start_index = tables.size();

    // Build action list based on available tables
    std::vector<std::string> actions;
    if (!tables.empty()) {
        actions.push_back("Open selected table");
        actions.push_back("Create new table");
        actions.push_back("Delete a table");
        actions.push_back("View table metadata");
        actions.push_back("Exit");
    } else {
        actions.push_back("Create new table");
        actions.push_back("Exit");
    }

    for (size_t i = 0; i < actions.size(); ++i) {
        bool is_selected = (selected_index == action_start_index + static_cast<int>(i));

        char indicator = is_selected ? '>' : ' ';
        terminal_.write_char(row, 6, indicator);

        std::string display = actions[i];
        for (size_t j = 0; j < display.length(); ++j) {
            terminal_.write_char(row, 8 + j, display[j]);
        }

        row++;
    }

    // Instructions
    int instr_row = row + 2;
    std::string instr = "Use arrow keys to navigate, Enter to select, Q to quit";
    int instr_col = (terminal_.cols() - instr.length()) / 2;
    for (size_t i = 0; i < instr.length(); ++i) {
        terminal_.write_char(instr_row, instr_col + i, instr[i]);
    }
}

int TableSelectionMenu::normalize_selection(int index, const std::vector<std::string>& tables) const {
    int max_items = get_item_count(tables);
    if (max_items == 0) return 0;

    // Handle wrapping
    while (index < 0) {
        index += max_items;
    }
    while (index >= max_items) {
        index -= max_items;
    }

    return index;
}

int TableSelectionMenu::get_item_count(const std::vector<std::string>& tables) const {
    if (tables.empty()) {
        // Only "Create" and "Exit" actions
        return 2;
    } else {
        // Tables + actions (Open, Create, Delete, View Metadata, Exit)
        return tables.size() + 5;
    }
}

MenuAction TableSelectionMenu::index_to_action(int index, const std::vector<std::string>& tables) const {
    int action_start = tables.size();

    if (index < action_start) {
        // Table selected - default action is to open it
        return MenuAction::OPEN_TABLE;
    }

    // Adjust for action index
    int action_index = index - action_start;

    if (tables.empty()) {
        // Actions: Create, Exit
        if (action_index == 0) return MenuAction::CREATE_TABLE;
        return MenuAction::EXIT;
    } else {
        // Actions: Open, Create, Delete, View Metadata, Exit
        switch (action_index) {
            case 0: return MenuAction::OPEN_TABLE;
            case 1: return MenuAction::CREATE_TABLE;
            case 2: return MenuAction::DELETE_TABLE;
            case 3: return MenuAction::VIEW_METADATA;
            default: return MenuAction::EXIT;
        }
    }
}

std::optional<std::string> TableSelectionMenu::index_to_table(int index,
                                                              const std::vector<std::string>& tables) const {
    if (index >= 0 && index < static_cast<int>(tables.size())) {
        return tables[index];
    }
    return std::nullopt;
}

MenuResult TableSelectionMenu::run(const std::vector<std::string>& tables) {
    int selected_index = 0;
    bool running = true;
    bool needs_redraw = true;

    while (running) {
        if (needs_redraw) {
            render(tables, selected_index);
            terminal_.render();
            needs_redraw = false;
        }

        // Read keyboard input
        int key = terminal_.read_key();
        if (key >= 0) {
            if (key == Terminal::KEY_RESIZE) {
                // Terminal was resized - update dimensions and redraw
                terminal_.detect_size();
                needs_redraw = true;
            } else if (key == 12) {  // Ctrl-L - manual refresh
                terminal_.detect_size();
                needs_redraw = true;
            } else if (key == Terminal::KEY_UP_ARROW) {
                selected_index = normalize_selection(selected_index - 1, tables);
                needs_redraw = true;
            } else if (key == Terminal::KEY_DOWN_ARROW) {
                selected_index = normalize_selection(selected_index + 1, tables);
                needs_redraw = true;
            } else if (key == '\n' || key == '\r') {
                // Enter key - select current item
                MenuAction action = index_to_action(selected_index, tables);
                std::optional<std::string> table_name = std::nullopt;

                // If a table row is selected or "Open" action is selected, get the table
                if (selected_index < static_cast<int>(tables.size())) {
                    table_name = index_to_table(selected_index, tables);
                }

                return MenuResult{action, table_name};
            } else if (key == 'q' || key == 'Q' || key == 27) {  // Q or ESC
                return MenuResult{MenuAction::EXIT, std::nullopt};
            }
        }
    }

    return MenuResult{MenuAction::EXIT, std::nullopt};
}

}  // namespace datapainter
