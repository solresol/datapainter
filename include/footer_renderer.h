#pragma once

#include "terminal.h"

namespace datapainter {

// Renders the footer area showing cursor position, controls, and action buttons
class FooterRenderer {
public:
    FooterRenderer() = default;

    // Render the footer to the terminal
    // Parameters:
    //   terminal: Terminal buffer to render to
    //   cursor_x: Current cursor x position in data coordinates
    //   cursor_y: Current cursor y position in data coordinates
    //   x_min: Minimum valid x value
    //   x_max: Maximum valid x value
    //   y_min: Minimum valid y value
    //   y_max: Maximum valid y value
    //   focused_button: Which button has focus (for Tab navigation, 0-based)
    //                   0=none (viewport focused), >0 = specific button
    void render(Terminal& terminal, double cursor_x, double cursor_y,
                double x_min, double x_max, double y_min, double y_max,
                int focused_button);

private:
    // Format a coordinate value with appropriate precision
    std::string format_coord(double value) const;
};

}  // namespace datapainter
