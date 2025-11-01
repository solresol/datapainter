#pragma once

#include <string>
#include <vector>

namespace datapainter {

// Forward declarations
class Terminal;
class Viewport;

// Represents a tick mark position and label
struct TickMark {
    double value;
    std::string label;
};

// Axis rendering with smart tick placement
class AxisRenderer {
public:
    // Calculate tick step to prevent label collision
    // Returns a value of form 10^k × {1, 2, or 5}
    static double calculate_tick_step(double data_min, double data_max,
                                      int available_space);

    // Generate major tick marks
    static std::vector<TickMark> generate_major_ticks(double data_min,
                                                       double data_max,
                                                       double tick_step);

    // Generate minor tick marks (if spacing permits)
    static std::vector<double> generate_minor_ticks(double data_min,
                                                     double data_max,
                                                     double major_step);

    // Generate tenth tick marks (if >= 6 chars between majors)
    static std::vector<double> generate_tenth_ticks(double data_min,
                                                     double data_max,
                                                     double major_step);

    // Format tick label with appropriate precision
    // Uses scientific notation for |exponent| >= 4
    static std::string format_label(double value);

    // Calculate decimal place for major tick marks (log10 of range)
    static int calculate_decimal_places(double data_min, double data_max);

    // Render x-axis with tick marks and labels
    // axis_row: The row on which to draw the axis (typically bottom of content area)
    // start_col: Starting column for the axis
    // width: Width of the axis in characters
    // axis_name: Name to display for the axis (column name)
    void render_x_axis(Terminal& terminal, const Viewport& viewport,
                      int axis_row, int start_col, int width,
                      const std::string& axis_name);

    // Render y-axis with tick marks and labels
    // axis_col: The column on which to draw the axis (typically left of content area)
    // start_row: Starting row for the axis
    // height: Height of the axis in characters
    // axis_name: Name to display for the axis (column name)
    void render_y_axis(Terminal& terminal, const Viewport& viewport,
                      int axis_col, int start_row, int height,
                      const std::string& axis_name);

    // Render zero bars (Cartesian axes at x=0 and y=0)
    // Only renders when show_zero_bars is true AND the zero line is in viewport
    // start_row: Starting row for the content area
    // start_col: Starting column for the content area
    // height: Height of the content area
    // width: Width of the content area
    // show_zero_bars: Whether to show zero bars (from --show-zero-bars flag)
    void render_zero_bars(Terminal& terminal, const Viewport& viewport,
                         int start_row, int start_col, int height, int width,
                         bool show_zero_bars);

private:
    // Helper to find nearest "nice" number (1, 2, or 5 times power of 10)
    static double round_to_nice(double value);
};

} // namespace datapainter
