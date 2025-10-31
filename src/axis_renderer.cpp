#include "axis_renderer.h"
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

} // namespace datapainter
