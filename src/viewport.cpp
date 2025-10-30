#include "viewport.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace datapainter {

Viewport::Viewport(double data_x_min, double data_x_max,
                   double data_y_min, double data_y_max,
                   int screen_height, int screen_width)
    : data_x_min_(data_x_min)
    , data_x_max_(data_x_max)
    , data_y_min_(data_y_min)
    , data_y_max_(data_y_max)
    , valid_x_min_(-std::numeric_limits<double>::infinity())
    , valid_x_max_(std::numeric_limits<double>::infinity())
    , valid_y_min_(-std::numeric_limits<double>::infinity())
    , valid_y_max_(std::numeric_limits<double>::infinity())
    , screen_height_(screen_height)
    , screen_width_(screen_width) {}

Viewport::Viewport(double data_x_min, double data_x_max,
                   double data_y_min, double data_y_max,
                   double valid_x_min, double valid_x_max,
                   double valid_y_min, double valid_y_max,
                   int screen_height, int screen_width)
    : data_x_min_(data_x_min)
    , data_x_max_(data_x_max)
    , data_y_min_(data_y_min)
    , data_y_max_(data_y_max)
    , valid_x_min_(valid_x_min)
    , valid_x_max_(valid_x_max)
    , valid_y_min_(valid_y_min)
    , valid_y_max_(valid_y_max)
    , screen_height_(screen_height)
    , screen_width_(screen_width) {}

DataCoord Viewport::screen_to_data(const ScreenCoord& screen) const {
    // Calculate data per pixel
    double data_width = data_x_max_ - data_x_min_;
    double data_height = data_y_max_ - data_y_min_;

    // Map screen col to data x (left to right)
    double x = data_x_min_ + (screen.col * data_width) / (screen_width_ - 1);

    // Map screen row to data y (top to bottom, but y increases upward in data)
    double y = data_y_max_ - (screen.row * data_height) / (screen_height_ - 1);

    return DataCoord{x, y};
}

std::optional<ScreenCoord> Viewport::data_to_screen(const DataCoord& data) const {
    // Check if visible
    if (!is_visible(data)) {
        return std::nullopt;
    }

    // Calculate data per pixel
    double data_width = data_x_max_ - data_x_min_;
    double data_height = data_y_max_ - data_y_min_;

    // Map data x to screen col
    int col = static_cast<int>(std::round(
        (data.x - data_x_min_) * (screen_width_ - 1) / data_width
    ));

    // Map data y to screen row (y decreases as row increases)
    int row = static_cast<int>(std::round(
        (data_y_max_ - data.y) * (screen_height_ - 1) / data_height
    ));

    // Clamp to screen bounds
    col = std::max(0, std::min(col, screen_width_ - 1));
    row = std::max(0, std::min(row, screen_height_ - 1));

    return ScreenCoord{row, col};
}

bool Viewport::is_visible(const DataCoord& data) const {
    return data.x >= data_x_min_ && data.x <= data_x_max_ &&
           data.y >= data_y_min_ && data.y <= data_y_max_;
}

DataCoord Viewport::round_to_cell(const DataCoord& data) const {
    // Convert to screen and back to get rounded data coordinates
    auto screen = data_to_screen(data);
    if (!screen.has_value()) {
        // If outside viewport, just return original
        return data;
    }
    return screen_to_data(*screen);
}

void Viewport::clamp_to_valid_ranges() {
    // Clamp the viewport to valid ranges
    // If viewport is larger than valid range, center it on the valid range
    double viewport_x_width = data_x_max_ - data_x_min_;
    double viewport_y_height = data_y_max_ - data_y_min_;
    double valid_x_width = valid_x_max_ - valid_x_min_;
    double valid_y_height = valid_y_max_ - valid_y_min_;

    // Handle X axis
    if (viewport_x_width >= valid_x_width) {
        // Viewport is larger than or equal to valid range - show entire valid range
        data_x_min_ = valid_x_min_;
        data_x_max_ = valid_x_max_;
    } else {
        // Viewport is smaller - clamp to stay within valid range
        if (data_x_min_ < valid_x_min_) {
            data_x_min_ = valid_x_min_;
            data_x_max_ = valid_x_min_ + viewport_x_width;
        }
        if (data_x_max_ > valid_x_max_) {
            data_x_max_ = valid_x_max_;
            data_x_min_ = valid_x_max_ - viewport_x_width;
        }
    }

    // Handle Y axis
    if (viewport_y_height >= valid_y_height) {
        // Viewport is larger than or equal to valid range - show entire valid range
        data_y_min_ = valid_y_min_;
        data_y_max_ = valid_y_max_;
    } else {
        // Viewport is smaller - clamp to stay within valid range
        if (data_y_min_ < valid_y_min_) {
            data_y_min_ = valid_y_min_;
            data_y_max_ = valid_y_min_ + viewport_y_height;
        }
        if (data_y_max_ > valid_y_max_) {
            data_y_max_ = valid_y_max_;
            data_y_min_ = valid_y_max_ - viewport_y_height;
        }
    }
}

void Viewport::zoom_in(const DataCoord& center) {
    // Calculate new viewport size (halve current size)
    double new_x_range = (data_x_max_ - data_x_min_) / 2.0;
    double new_y_range = (data_y_max_ - data_y_min_) / 2.0;

    double half_x_range = new_x_range / 2.0;
    double half_y_range = new_y_range / 2.0;

    // Start with cursor position as center
    double center_x = center.x;
    double center_y = center.y;

    // Adjust center_x to avoid showing too much forbidden area on X axis
    double valid_x_range = valid_x_max_ - valid_x_min_;
    if (new_x_range >= valid_x_range) {
        // New viewport is larger than or equal to valid range
        // Center on the valid range midpoint
        center_x = (valid_x_min_ + valid_x_max_) / 2.0;
    } else {
        // Check if centering on cursor would exceed valid_x_min
        if (center_x - half_x_range < valid_x_min_) {
            center_x = valid_x_min_ + half_x_range;
        }
        // Check if centering on cursor would exceed valid_x_max
        if (center_x + half_x_range > valid_x_max_) {
            center_x = valid_x_max_ - half_x_range;
        }
    }

    // Adjust center_y to avoid showing too much forbidden area on Y axis
    double valid_y_range = valid_y_max_ - valid_y_min_;
    if (new_y_range >= valid_y_range) {
        // New viewport is larger than or equal to valid range
        // Center on the valid range midpoint
        center_y = (valid_y_min_ + valid_y_max_) / 2.0;
    } else {
        // Check if centering on cursor would exceed valid_y_min
        if (center_y - half_y_range < valid_y_min_) {
            center_y = valid_y_min_ + half_y_range;
        }
        // Check if centering on cursor would exceed valid_y_max
        if (center_y + half_y_range > valid_y_max_) {
            center_y = valid_y_max_ - half_y_range;
        }
    }

    // Apply zoom with adjusted center
    data_x_min_ = center_x - half_x_range;
    data_x_max_ = center_x + half_x_range;
    data_y_min_ = center_y - half_y_range;
    data_y_max_ = center_y + half_y_range;

    // Final clamp to valid ranges (for safety)
    clamp_to_valid_ranges();
}

void Viewport::zoom_out(const DataCoord& center) {
    // Double the viewport size, centered on given point
    double x_range = data_x_max_ - data_x_min_;
    double y_range = data_y_max_ - data_y_min_;

    data_x_min_ = center.x - x_range;
    data_x_max_ = center.x + x_range;
    data_y_min_ = center.y - y_range;
    data_y_max_ = center.y + y_range;

    // Clamp to valid ranges
    clamp_to_valid_ranges();
}

void Viewport::pan_right() {
    // Pan right by 1/4 of viewport width
    double pan_amount = (data_x_max_ - data_x_min_) * 0.25;
    data_x_min_ += pan_amount;
    data_x_max_ += pan_amount;
    clamp_to_valid_ranges();
}

void Viewport::pan_left() {
    // Pan left by 1/4 of viewport width
    double pan_amount = (data_x_max_ - data_x_min_) * 0.25;
    data_x_min_ -= pan_amount;
    data_x_max_ -= pan_amount;
    clamp_to_valid_ranges();
}

void Viewport::pan_up() {
    // Pan up by 1/4 of viewport height
    double pan_amount = (data_y_max_ - data_y_min_) * 0.25;
    data_y_min_ += pan_amount;
    data_y_max_ += pan_amount;
    clamp_to_valid_ranges();
}

void Viewport::pan_down() {
    // Pan down by 1/4 of viewport height
    double pan_amount = (data_y_max_ - data_y_min_) * 0.25;
    data_y_min_ -= pan_amount;
    data_y_max_ -= pan_amount;
    clamp_to_valid_ranges();
}

void Viewport::zoom_to_fit_all() {
    // Placeholder implementation: just ensure viewport is valid
    // Real implementation would need actual data bounds passed in
    // For now, reset to a reasonable default if needed
    if (data_x_min_ >= data_x_max_) {
        data_x_min_ = -1.0;
        data_x_max_ = 1.0;
    }
    if (data_y_min_ >= data_y_max_) {
        data_y_min_ = -1.0;
        data_y_max_ = 1.0;
    }
}

} // namespace datapainter
