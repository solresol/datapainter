#pragma once

#include <optional>

namespace datapainter {

// Represents screen coordinates (row, col)
struct ScreenCoord {
    int row;
    int col;
};

// Represents data coordinates (x, y)
struct DataCoord {
    double x;
    double y;
};

// Viewport manages the mapping between screen space and data space
class Viewport {
public:
    // Create viewport with data bounds and screen dimensions
    Viewport(double data_x_min, double data_x_max,
             double data_y_min, double data_y_max,
             int screen_height, int screen_width);

    // Create viewport with data bounds, valid ranges, and screen dimensions
    Viewport(double data_x_min, double data_x_max,
             double data_y_min, double data_y_max,
             double valid_x_min, double valid_x_max,
             double valid_y_min, double valid_y_max,
             int screen_height, int screen_width);

    // Coordinate transformations
    DataCoord screen_to_data(const ScreenCoord& screen) const;
    std::optional<ScreenCoord> data_to_screen(const DataCoord& data) const;

    // Check if data point is visible
    bool is_visible(const DataCoord& data) const;

    // Round data coordinate to nearest screen cell
    DataCoord round_to_cell(const DataCoord& data) const;

    // Zoom operations (centered on given data coordinate)
    void zoom_in(const DataCoord& center);
    void zoom_out(const DataCoord& center);
    void zoom_to_fit_all();  // Fit all data with 10% padding

    // Pan operations
    void pan_right();
    void pan_left();
    void pan_up();
    void pan_down();

    // Getters for bounds
    double data_x_min() const { return data_x_min_; }
    double data_x_max() const { return data_x_max_; }
    double data_y_min() const { return data_y_min_; }
    double data_y_max() const { return data_y_max_; }
    double valid_x_min() const { return valid_x_min_; }
    double valid_x_max() const { return valid_x_max_; }
    double valid_y_min() const { return valid_y_min_; }
    double valid_y_max() const { return valid_y_max_; }
    int screen_height() const { return screen_height_; }
    int screen_width() const { return screen_width_; }

private:
    void clamp_to_valid_ranges();

    double data_x_min_;
    double data_x_max_;
    double data_y_min_;
    double data_y_max_;
    double valid_x_min_;
    double valid_x_max_;
    double valid_y_min_;
    double valid_y_max_;
    int screen_height_;
    int screen_width_;
};

} // namespace datapainter
