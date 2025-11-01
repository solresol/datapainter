#include "axis_renderer.h"
#include "terminal.h"
#include "viewport.h"
#include <cmath>
#include <sstream>
#include <iomanip>

namespace datapainter {

double AxisRenderer::calculate_tick_step(double data_min, double data_max,
                                          int available_space) {
    double range = data_max - data_min;
    if (range <= 0.0) {
        return 1.0;
    }

    // Estimate how many ticks we can fit (assuming ~6 chars per label)
    int max_ticks = available_space / 6;
    if (max_ticks < 2) {
        max_ticks = 2;
    }

    // Calculate rough step size
    double rough_step = range / (max_ticks - 1);

    // Round to nice number
    return round_to_nice(rough_step);
}

std::vector<TickMark> AxisRenderer::generate_major_ticks(double data_min,
                                                          double data_max,
                                                          double tick_step) {
    std::vector<TickMark> ticks;

    if (tick_step <= 0.0) {
        return ticks;
    }

    // Find first tick at or before data_min
    double first_tick = std::floor(data_min / tick_step) * tick_step;

    // Generate ticks from first to last
    for (double value = first_tick; value <= data_max + tick_step * 0.5; value += tick_step) {
        TickMark tick;
        tick.value = value;
        tick.label = format_label(value);
        ticks.push_back(tick);
    }

    return ticks;
}

std::vector<double> AxisRenderer::generate_minor_ticks(double data_min,
                                                        double data_max,
                                                        double major_step) {
    std::vector<double> minors;

    if (major_step <= 0.0) {
        return minors;
    }

    // Generate 4 minor ticks between each major tick
    double minor_step = major_step / 5.0;

    double first_minor = std::floor(data_min / minor_step) * minor_step;

    for (double value = first_minor; value <= data_max; value += minor_step) {
        // Skip if this coincides with a major tick
        double remainder = std::fmod(std::abs(value), major_step);
        if (remainder > minor_step * 0.1 && remainder < major_step - minor_step * 0.1) {
            minors.push_back(value);
        }
    }

    return minors;
}

std::vector<double> AxisRenderer::generate_tenth_ticks(double data_min,
                                                        double data_max,
                                                        double major_step) {
    std::vector<double> tenths;

    if (major_step <= 0.0) {
        return tenths;
    }

    // Generate 9 tenth ticks between each major tick (10 divisions total)
    double tenth_step = major_step / 10.0;
    double minor_step = major_step / 5.0;

    double first_tenth = std::floor(data_min / tenth_step) * tenth_step;

    for (double value = first_tenth; value <= data_max; value += tenth_step) {
        // Skip if this coincides with a major tick
        double major_remainder = std::fmod(std::abs(value), major_step);
        bool at_major = major_remainder < tenth_step * 0.1 ||
                        major_remainder > major_step - tenth_step * 0.1;

        // Skip if this coincides with a minor tick
        double minor_remainder = std::fmod(std::abs(value), minor_step);
        bool at_minor = minor_remainder < tenth_step * 0.1 ||
                        minor_remainder > minor_step - tenth_step * 0.1;

        if (!at_major && !at_minor) {
            tenths.push_back(value);
        }
    }

    return tenths;
}

std::string AxisRenderer::format_label(double value) {
    // Handle zero specially
    if (std::abs(value) < 1e-10) {
        return "0";
    }

    // Determine if we need scientific notation (|exponent| >= 4)
    double abs_value = std::abs(value);
    double log_value = std::log10(abs_value);
    int exponent = static_cast<int>(std::floor(log_value));

    if (std::abs(exponent) >= 4) {
        // Use scientific notation
        std::ostringstream oss;
        oss << std::scientific << std::setprecision(1) << value;
        return oss.str();
    }

    // Use fixed notation
    std::ostringstream oss;

    // Determine precision based on magnitude
    int precision = 0;
    if (abs_value < 1.0) {
        precision = std::abs(exponent) + 1;
    } else if (abs_value < 10.0) {
        precision = 1;
    }

    oss << std::fixed << std::setprecision(precision) << value;
    std::string result = oss.str();

    // Remove trailing zeros after decimal point
    if (result.find('.') != std::string::npos) {
        size_t last_nonzero = result.find_last_not_of('0');
        if (last_nonzero != std::string::npos) {
            if (result[last_nonzero] == '.') {
                result.erase(last_nonzero);
            } else {
                result.erase(last_nonzero + 1);
            }
        }
    }

    return result;
}

int AxisRenderer::calculate_decimal_places(double data_min, double data_max) {
    double range = data_max - data_min;
    if (range <= 0.0) {
        return 0;
    }

    return static_cast<int>(std::floor(std::log10(range)));
}

double AxisRenderer::round_to_nice(double value) {
    if (value <= 0.0) {
        return 1.0;
    }

    // Get order of magnitude
    double exponent = std::floor(std::log10(value));
    double power_of_10 = std::pow(10.0, exponent);

    // Get mantissa (value / 10^exponent)
    double mantissa = value / power_of_10;

    // Round mantissa to 1, 2, or 5
    double nice_mantissa;
    if (mantissa <= 1.5) {
        nice_mantissa = 1.0;
    } else if (mantissa <= 3.0) {
        nice_mantissa = 2.0;
    } else if (mantissa <= 7.0) {
        nice_mantissa = 5.0;
    } else {
        nice_mantissa = 10.0;
    }

    return nice_mantissa * power_of_10;
}

void AxisRenderer::render_x_axis(Terminal& terminal, const Viewport& viewport,
                                 int axis_row, int start_col, int width,
                                 const std::string& axis_name) {
    // Suppress unused parameter warning
    (void)axis_name;

    // Calculate tick step for x-axis
    double data_min = viewport.data_x_min();
    double data_max = viewport.data_x_max();
    double tick_step = calculate_tick_step(data_min, data_max, width);

    // Generate major ticks
    auto major_ticks = generate_major_ticks(data_min, data_max, tick_step);

    // Draw a horizontal line for the axis
    for (int col = start_col; col < start_col + width; ++col) {
        terminal.write_char(axis_row, col, '-');
    }

    // Draw tick marks and labels
    for (const auto& tick : major_ticks) {
        // Convert data coordinate to screen coordinate
        DataCoord data{tick.value, 0.0};  // y doesn't matter for x-axis
        auto screen_opt = viewport.data_to_screen(data);

        if (screen_opt.has_value()) {
            auto screen = screen_opt.value();
            int tick_col = start_col + screen.col;

            // Check if tick is within axis bounds
            if (tick_col >= start_col && tick_col < start_col + width) {
                // Draw tick mark
                terminal.write_char(axis_row, tick_col, '|');

                // Draw label (centered on tick)
                // Place label characters around the tick position
                int label_len = tick.label.length();
                int label_start = tick_col - label_len / 2;

                for (size_t i = 0; i < tick.label.length(); ++i) {
                    int label_col = label_start + i;
                    if (label_col >= start_col && label_col < start_col + width &&
                        label_col != tick_col) {  // Don't overwrite tick mark
                        terminal.write_char(axis_row, label_col, tick.label[i]);
                    }
                }
            }
        }
    }
}

void AxisRenderer::render_y_axis(Terminal& terminal, const Viewport& viewport,
                                 int axis_col, int start_row, int height,
                                 const std::string& axis_name) {
    // Suppress unused parameter warning
    (void)axis_name;

    // Calculate tick step for y-axis
    double data_min = viewport.data_y_min();
    double data_max = viewport.data_y_max();
    double tick_step = calculate_tick_step(data_min, data_max, height);

    // Generate major ticks
    auto major_ticks = generate_major_ticks(data_min, data_max, tick_step);

    // Draw a vertical line for the axis
    for (int row = start_row; row < start_row + height; ++row) {
        terminal.write_char(row, axis_col, '|');
    }

    // Draw tick marks and labels
    for (const auto& tick : major_ticks) {
        // Convert data coordinate to screen coordinate
        DataCoord data{0.0, tick.value};  // x doesn't matter for y-axis
        auto screen_opt = viewport.data_to_screen(data);

        if (screen_opt.has_value()) {
            auto screen = screen_opt.value();
            int tick_row = start_row + screen.row;

            // Check if tick is within axis bounds
            if (tick_row >= start_row && tick_row < start_row + height) {
                // Draw tick mark
                terminal.write_char(tick_row, axis_col, '-');

                // Draw label (to the left of the axis)
                // Place label characters to the left of the tick
                int label_len = tick.label.length();
                int label_start = axis_col - label_len - 1;

                if (label_start >= 0) {
                    for (size_t i = 0; i < tick.label.length(); ++i) {
                        int label_col = label_start + i;
                        if (label_col >= 0) {
                            terminal.write_char(tick_row, label_col, tick.label[i]);
                        }
                    }
                }
            }
        }
    }
}

void AxisRenderer::render_zero_bars(Terminal& terminal, const Viewport& viewport,
                                    int start_row, int start_col, int height, int width,
                                    bool show_zero_bars) {
    if (!show_zero_bars) {
        return;  // Zero bars disabled
    }

    // Check if x=0 is within the viewport data range
    double x_min = viewport.data_x_min();
    double x_max = viewport.data_x_max();
    bool x_zero_in_viewport = (x_min <= 0.0 && x_max >= 0.0);

    // Check if y=0 is within the viewport data range
    double y_min = viewport.data_y_min();
    double y_max = viewport.data_y_max();
    bool y_zero_in_viewport = (y_min <= 0.0 && y_max >= 0.0);

    // Draw vertical line at x=0 if it's in the viewport
    if (x_zero_in_viewport) {
        DataCoord x_zero{0.0, 0.0};
        auto screen_opt = viewport.data_to_screen(x_zero);

        if (screen_opt.has_value()) {
            auto screen = screen_opt.value();
            int zero_col = start_col + screen.col;

            // Draw vertical line through all rows
            if (zero_col >= start_col && zero_col < start_col + width) {
                for (int row = start_row; row < start_row + height; ++row) {
                    terminal.write_char(row, zero_col, '|');
                }
            }
        }
    }

    // Draw horizontal line at y=0 if it's in the viewport
    if (y_zero_in_viewport) {
        DataCoord y_zero{0.0, 0.0};
        auto screen_opt = viewport.data_to_screen(y_zero);

        if (screen_opt.has_value()) {
            auto screen = screen_opt.value();
            int zero_row = start_row + screen.row;

            // Draw horizontal line through all columns
            if (zero_row >= start_row && zero_row < start_row + height) {
                for (int col = start_col; col < start_col + width; ++col) {
                    terminal.write_char(zero_row, col, '-');
                }
            }
        }
    }

    // If both x=0 and y=0 are in viewport, mark the origin with a special character
    if (x_zero_in_viewport && y_zero_in_viewport) {
        DataCoord origin{0.0, 0.0};
        auto screen_opt = viewport.data_to_screen(origin);

        if (screen_opt.has_value()) {
            auto screen = screen_opt.value();
            int zero_row = start_row + screen.row;
            int zero_col = start_col + screen.col;

            if (zero_row >= start_row && zero_row < start_row + height &&
                zero_col >= start_col && zero_col < start_col + width) {
                terminal.write_char(zero_row, zero_col, '+');
            }
        }
    }
}

} // namespace datapainter
