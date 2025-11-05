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
    //   zoom_percent: Current zoom level as percentage (e.g., 100.0 = 100%)
    //   pan_step_percent: Current pan step as percentage of viewport (e.g., 10.0 = 10%)
    void render(Terminal& terminal, int rows, int cols, double zoom_percent, double pan_step_percent);

private:
    // Get all help lines to display
    std::vector<std::string> get_help_lines(double zoom_percent, double pan_step_percent) const;

    // Center text within given width
    std::string center_text(const std::string& text, int width) const;
};

}  // namespace datapainter
