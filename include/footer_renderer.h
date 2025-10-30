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
    //   vp_x_min: Current viewport minimum x
    //   vp_x_max: Current viewport maximum x
    //   vp_y_min: Current viewport minimum y
    //   vp_y_max: Current viewport maximum y
    //   focused_button: Which button has focus (for Tab navigation, 0-based)
    //                   0=none (viewport focused), >0 = specific button
    void render(Terminal& terminal, double cursor_x, double cursor_y,
                double x_min, double x_max, double y_min, double y_max,
                double vp_x_min, double vp_x_max, double vp_y_min, double vp_y_max,
                int focused_button);

private:
    // Calculate appropriate precision for displaying coordinates
    // based on viewport size and screen dimensions
    int calculate_precision(double range, int screen_size) const;

    // Format a coordinate value with specified precision
    std::string format_coord(double value, int precision) const;
};

}  // namespace datapainter
