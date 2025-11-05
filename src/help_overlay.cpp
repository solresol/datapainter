#include "help_overlay.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace datapainter {

void HelpOverlay::render(Terminal& terminal, int rows, int cols, double zoom_percent, double pan_step_percent) {
    // Clear the entire screen
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            terminal.write_char(row, col, ' ');
        }
    }

    // Get help content
    auto help_lines = get_help_lines(zoom_percent, pan_step_percent);

    // Calculate starting row to center vertically
    int content_height = static_cast<int>(help_lines.size());
    int start_row = std::max(0, (rows - content_height) / 2);

    // Render each line centered horizontally
    for (size_t i = 0; i < help_lines.size() && (start_row + static_cast<int>(i)) < rows; ++i) {
        std::string line = center_text(help_lines[i], cols);

        int row = start_row + static_cast<int>(i);
        for (size_t col = 0; col < line.length() && static_cast<int>(col) < cols; ++col) {
            terminal.write_char(row, static_cast<int>(col), line[col]);
        }
    }
}

std::vector<std::string> HelpOverlay::get_help_lines(double zoom_percent, double pan_step_percent) const {
    // Format zoom and pan step info
    std::ostringstream zoom_line;
    zoom_line << "|  Current Zoom: " << std::fixed << std::setprecision(0) << zoom_percent << "%";
    // Pad to 54 chars (to match border width)
    while (zoom_line.str().length() < 54) {
        zoom_line << " ";
    }
    zoom_line << "|";

    std::ostringstream pan_line;
    pan_line << "|  Pan Step: " << std::fixed << std::setprecision(0) << pan_step_percent << "% of viewport";
    // Pad to 54 chars
    while (pan_line.str().length() < 54) {
        pan_line << " ";
    }
    pan_line << "|";

    return {
        "+======================================================+",
        "|                  DATAPAINTER HELP                    |",
        "+======================================================+",
        "|                                                      |",
        zoom_line.str(),
        pan_line.str(),
        "|                                                      |",
        "|  NAVIGATION:                                         |",
        "|    Arrow keys - Move cursor                          |",
        "|    Tab        - Navigate header fields and buttons   |",
        "|                                                      |",
        "|  POINT EDITING:                                      |",
        "|    x         - Create x point at cursor              |",
        "|    o         - Create o point at cursor              |",
        "|    Space     - Delete all points under cursor        |",
        "|    Shift+X   - Convert o points to x under cursor    |",
        "|    Shift+O   - Convert x points to o under cursor    |",
        "|    g         - Flip points (x<->o) under cursor      |",
        "|                                                      |",
        "|  ZOOM & VIEW:                                        |",
        "|    +         - Zoom in                               |",
        "|    -         - Zoom out                              |",
        "|    =         - Full viewport (fit all data)          |",
        "|    #         - Toggle tabular view                   |",
        "|                                                      |",
        "|  UNDO/SAVE/QUIT:                                     |",
        "|    u         - Undo last action                      |",
        "|    s         - Save changes to database              |",
        "|    q         - Quit (prompts if unsaved changes)     |",
        "|                                                      |",
        "|  OTHER:                                              |",
        "|    r         - Generate random points                |",
        "|    ?         - Show this help                        |",
        "|    k         - Dump full screen to stdout            |",
        "|    Shift+K   - Dump edit area to stdout              |",
        "|                                                      |",
        "+======================================================+",
        "|                                                      |",
        "|           Press any key to close help                |",
        "|                                                      |",
        "+======================================================+"
    };
}

std::string HelpOverlay::center_text(const std::string& text, int width) const {
    if (static_cast<int>(text.length()) >= width) {
        return text.substr(0, width);
    }

    int padding = (width - static_cast<int>(text.length())) / 2;
    return std::string(padding, ' ') + text;
}

}  // namespace datapainter
