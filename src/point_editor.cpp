#include "point_editor.h"
#include "metadata.h"
#include "unsaved_changes.h"
#include <cmath>
#include <optional>
#include <map>

namespace datapainter {

PointEditor::PointEditor(Database& db, const std::string& table_name)
    : db_(db), table_name_(table_name),
      x_meaning_(""), o_meaning_(""),
      x_min_(-10.0), x_max_(10.0),
      y_min_(-10.0), y_max_(10.0) {
    load_metadata();
}

void PointEditor::load_metadata() {
    MetadataManager mgr(db_);
    auto metadata = mgr.read(table_name_);
    if (metadata) {
        x_meaning_ = metadata->x_meaning;
        o_meaning_ = metadata->o_meaning;
        x_min_ = metadata->valid_x_min.value_or(-10.0);
        x_max_ = metadata->valid_x_max.value_or(10.0);
        y_min_ = metadata->valid_y_min.value_or(-10.0);
        y_max_ = metadata->valid_y_max.value_or(10.0);
    }
}

bool PointEditor::create_point(double x, double y, char type) {
    // Check if point is within valid range
    if (x < x_min_ || x > x_max_ || y < y_min_ || y > y_max_) {
        return false;
    }

    // Determine target value based on type
    std::string target;
    if (type == 'x' || type == 'X') {
        target = x_meaning_;
    } else if (type == 'o' || type == 'O') {
        target = o_meaning_;
    } else {
        return false;
    }

    // Record in unsaved changes ONLY (don't insert into database yet)
    // The point will be inserted when the user saves
    UnsavedChanges uc(db_);
    auto change_id = uc.record_insert(table_name_, x, y, target);

    return change_id.has_value();
}

int PointEditor::delete_points_at_cursor(double cursor_x, double cursor_y, double cell_size) {
    auto points = get_points_at_cursor(cursor_x, cursor_y, cell_size);

    UnsavedChanges uc(db_);

    for (const auto& point : points) {
        if (point.id < 0) {
            // This is an unsaved insert (negative ID = -change_id)
            // Mark the original insert as inactive (cancels the insert)
            int change_id = -point.id;  // Convert back to positive change ID
            uc.mark_change_inactive(change_id);
        } else {
            // Record deletion in unsaved changes ONLY (don't delete from database yet)
            uc.record_delete(table_name_, point.id, point.x, point.y, point.target);
        }
    }

    return points.size();
}

int PointEditor::convert_points_at_cursor(double cursor_x, double cursor_y,
                                          double cell_size, char to_type) {
    auto points = get_points_at_cursor(cursor_x, cursor_y, cell_size);

    // Determine target values
    std::string to_target;
    std::string from_target;
    if (to_type == 'x' || to_type == 'X') {
        to_target = x_meaning_;
        from_target = o_meaning_;
    } else if (to_type == 'o' || to_type == 'O') {
        to_target = o_meaning_;
        from_target = x_meaning_;
    } else {
        return 0;
    }

    UnsavedChanges uc(db_);

    int converted = 0;
    for (const auto& point : points) {
        // Only convert points that are currently the opposite type
        if (point.target == from_target) {
            // Record update in unsaved changes ONLY (don't update database yet)
            uc.record_update(table_name_, point.id, point.target, to_target);
            converted++;
        }
    }

    return converted;
}

int PointEditor::flip_points_at_cursor(double cursor_x, double cursor_y, double cell_size) {
    auto points = get_points_at_cursor(cursor_x, cursor_y, cell_size);

    UnsavedChanges uc(db_);

    for (const auto& point : points) {
        // Determine new target (flip)
        std::string new_target;
        if (point.target == x_meaning_) {
            new_target = o_meaning_;
        } else {
            new_target = x_meaning_;
        }

        // Record update in unsaved changes ONLY (don't update database yet)
        uc.record_update(table_name_, point.id, point.target, new_target);
    }

    return points.size();
}

std::vector<DataPoint> PointEditor::get_points_at_cursor(double cursor_x, double cursor_y,
                                                         double cell_size) {
    // Round cursor to cell center
    double cell_x = round_to_cell(cursor_x, cell_size);
    double cell_y = round_to_cell(cursor_y, cell_size);

    // Query points within the cell
    double half_cell = cell_size / 2.0;
    double x_min = cell_x - half_cell;
    double x_max = cell_x + half_cell;
    double y_min = cell_y - half_cell;
    double y_max = cell_y + half_cell;

    DataTable dt(db_, table_name_);
    auto all_points = dt.query_viewport(x_min, x_max, y_min, y_max);

    // Load unsaved changes and apply them
    UnsavedChanges uc(db_);
    auto unsaved_changes = uc.get_changes(table_name_);

    // Build maps to track unsaved changes
    std::map<int, bool> deleted_ids;
    std::map<int, std::string> updated_targets;

    for (const auto& change : unsaved_changes) {
        if (!change.is_active) continue;

        if (change.action == "delete" && change.data_id.has_value()) {
            deleted_ids[change.data_id.value()] = true;
        } else if (change.action == "update" && change.data_id.has_value() && change.new_target.has_value()) {
            updated_targets[change.data_id.value()] = change.new_target.value();
        }
    }

    // Filter database points: apply deletions and updates, check cell membership
    std::vector<DataPoint> result;
    for (auto point : all_points) {
        // Skip if deleted
        if (deleted_ids.count(point.id) > 0) {
            continue;
        }

        // Apply target update if any
        if (updated_targets.count(point.id) > 0) {
            point.target = updated_targets[point.id];
        }

        // Check if point rounds to the same cell
        double point_cell_x = round_to_cell(point.x, cell_size);
        double point_cell_y = round_to_cell(point.y, cell_size);

        if (std::abs(point_cell_x - cell_x) < 0.001 &&
            std::abs(point_cell_y - cell_y) < 0.001) {
            result.push_back(point);
        }
    }

    // Add inserted points from unsaved changes that fall in this cell
    for (const auto& change : unsaved_changes) {
        if (!change.is_active) continue;

        if (change.action == "insert" && change.x.has_value() && change.y.has_value() && change.new_target.has_value()) {
            double x = change.x.value();
            double y = change.y.value();

            // Check if inserted point is within cell bounds
            if (x >= x_min && x <= x_max && y >= y_min && y <= y_max) {
                double point_cell_x = round_to_cell(x, cell_size);
                double point_cell_y = round_to_cell(y, cell_size);

                if (std::abs(point_cell_x - cell_x) < 0.001 &&
                    std::abs(point_cell_y - cell_y) < 0.001) {
                    // Create a pseudo-DataPoint for this unsaved insert
                    // Use a negative ID to indicate it's not in the database yet
                    DataPoint pseudo_point;
                    pseudo_point.id = -change.id;  // Negative to distinguish from DB points
                    pseudo_point.x = x;
                    pseudo_point.y = y;
                    pseudo_point.target = change.new_target.value();
                    result.push_back(pseudo_point);
                }
            }
        }
    }

    return result;
}

double PointEditor::round_to_cell(double coord, double cell_size) const {
    return std::floor(coord / cell_size) * cell_size + cell_size / 2.0;
}

}  // namespace datapainter
