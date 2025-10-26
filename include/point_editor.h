#pragma once

#include "database.h"
#include "data_table.h"
#include <vector>
#include <string>

namespace datapainter {

// Manages point creation, deletion, and conversion operations
class PointEditor {
public:
    // Constructor
    // Parameters:
    //   db: Database connection
    //   table_name: Name of the data table to edit
    PointEditor(Database& db, const std::string& table_name);

    // Create a new point at the specified position
    // Parameters:
    //   x: X coordinate
    //   y: Y coordinate
    //   type: 'x' or 'o' to determine target value
    // Returns: true if point was created, false if outside valid range
    bool create_point(double x, double y, char type);

    // Delete all points at cursor position (within same screen cell)
    // Parameters:
    //   cursor_x: Cursor x position
    //   cursor_y: Cursor y position
    //   cell_size: Size of screen cell for rounding
    // Returns: Number of points deleted
    int delete_points_at_cursor(double cursor_x, double cursor_y, double cell_size);

    // Convert all points of opposite type to specified type at cursor
    // Parameters:
    //   cursor_x: Cursor x position
    //   cursor_y: Cursor y position
    //   cell_size: Size of screen cell for rounding
    //   to_type: 'x' or 'o' - target type to convert TO
    // Returns: Number of points converted
    int convert_points_at_cursor(double cursor_x, double cursor_y, double cell_size, char to_type);

    // Flip all points at cursor (x↔o)
    // Parameters:
    //   cursor_x: Cursor x position
    //   cursor_y: Cursor y position
    //   cell_size: Size of screen cell for rounding
    // Returns: Number of points flipped
    int flip_points_at_cursor(double cursor_x, double cursor_y, double cell_size);

    // Get all points at cursor position
    // Parameters:
    //   cursor_x: Cursor x position
    //   cursor_y: Cursor y position
    //   cell_size: Size of screen cell for rounding
    // Returns: Vector of points at cursor
    std::vector<DataPoint> get_points_at_cursor(double cursor_x, double cursor_y,
                                                double cell_size);

private:
    Database& db_;
    std::string table_name_;
    std::string x_meaning_;
    std::string o_meaning_;
    double x_min_;
    double x_max_;
    double y_min_;
    double y_max_;

    // Load metadata for the table
    void load_metadata();

    // Round coordinate to cell center
    double round_to_cell(double coord, double cell_size) const;
};

}  // namespace datapainter
