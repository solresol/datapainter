#pragma once

#include <string>
#include <vector>

namespace datapainter {

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

    // Format tick label with appropriate precision
    // Uses scientific notation for |exponent| >= 4
    static std::string format_label(double value);

    // Calculate decimal place for major tick marks (log10 of range)
    static int calculate_decimal_places(double data_min, double data_max);

private:
    // Helper to find nearest "nice" number (1, 2, or 5 times power of 10)
    static double round_to_nice(double value);
};

} // namespace datapainter
