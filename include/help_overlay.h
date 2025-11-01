#pragma once

#include "terminal.h"
#include <string>
#include <vector>

namespace datapainter {

// Displays help overlay showing keyboard shortcuts
class HelpOverlay {
public:
    HelpOverlay() = default;

    // Render help overlay to terminal
    // Parameters:
    //   terminal: Terminal buffer to render to
    //   rows: Terminal height in rows
    //   cols: Terminal width in columns
    void render(Terminal& terminal, int rows, int cols);

private:
    // Get all help lines to display
    std::vector<std::string> get_help_lines() const;

    // Center text within given width
    std::string center_text(const std::string& text, int width) const;
};

}  // namespace datapainter
