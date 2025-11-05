#include "table_view.h"
#include "unsaved_changes.h"
#include "data_table.h"
#include <sqlite3.h>
#include <sstream>
#include <cmath>
#include <map>

namespace datapainter {

TableView::TableView(Database& db, const std::string& table_name,
                     double x_min, double x_max, double y_min, double y_max)
    : db_(db), table_name_(table_name), current_row_(0), cached_row_count_(0) {

    // Build initial filter from viewport bounds
    std::ostringstream oss;
    if (x_min > -1e9 || x_max < 1e9 || y_min > -1e9 || y_max < 1e9) {
        oss << "x >= " << x_min << " AND x <= " << x_max
            << " AND y >= " << y_min << " AND y <= " << y_max;
        filter_ = oss.str();
    } else {
        filter_ = "";
    }

    refresh_row_count();
}

std::string TableView::build_query() const {
    std::ostringstream oss;
    oss << "SELECT id, x, y, target FROM " << table_name_;
    if (!filter_.empty()) {
        oss << " WHERE " << filter_;
    }
    oss << " ORDER BY id";
    return oss.str();
}

void TableView::refresh_row_count() {
    std::ostringstream oss;
    oss << "SELECT COUNT(*) FROM " << table_name_;
    if (!filter_.empty()) {
        oss << " WHERE " << filter_;
    }

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_.connection(), oss.str().c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        cached_row_count_ = 0;
        return;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        cached_row_count_ = sqlite3_column_int(stmt, 0);
    } else {
        cached_row_count_ = 0;
    }

    sqlite3_finalize(stmt);
}

std::vector<TableRow> TableView::get_visible_rows() const {
    std::vector<TableRow> rows;

    // First get all rows from database
    sqlite3_stmt* stmt = nullptr;
    std::string query = build_query();
    int rc = sqlite3_prepare_v2(db_.connection(), query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return rows;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        TableRow row;
        row.id = sqlite3_column_int(stmt, 0);
        row.x = sqlite3_column_double(stmt, 1);
        row.y = sqlite3_column_double(stmt, 2);
        const char* target_text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        row.target = target_text ? target_text : "";
        rows.push_back(row);
    }

    sqlite3_finalize(stmt);

    // Now apply unsaved changes
    UnsavedChanges uc(db_);
    auto changes = uc.get_changes(table_name_);

    // Build maps to track unsaved changes
    std::map<int, bool> deleted_ids;
    std::map<int, std::string> updated_targets;

    for (const auto& change : changes) {
        if (!change.is_active) continue;

        if (change.action == "delete" && change.data_id.has_value()) {
            deleted_ids[change.data_id.value()] = true;
        } else if (change.action == "update" && change.data_id.has_value() && change.new_target.has_value()) {
            updated_targets[change.data_id.value()] = change.new_target.value();
        }
    }

    // Filter database rows: apply deletions and updates
    std::vector<TableRow> result;
    for (auto row : rows) {
        // Skip if deleted
        if (deleted_ids.count(row.id) > 0) {
            continue;
        }

        // Apply target update if any
        if (updated_targets.count(row.id) > 0) {
            row.target = updated_targets[row.id];
        }

        result.push_back(row);
    }

    // Add inserted rows from unsaved changes
    for (const auto& change : changes) {
        if (!change.is_active) continue;

        if (change.action == "insert" && change.x.has_value() && change.y.has_value() && change.new_target.has_value()) {
            TableRow row;
            row.id = -change.id;  // Negative to distinguish from DB rows
            row.x = change.x.value();
            row.y = change.y.value();
            row.target = change.new_target.value();

            // Check if row matches current filter
            // For simplicity, we'll include all inserts for now
            // TODO: Apply filter to inserted rows
            result.push_back(row);
        }
    }

    return result;
}

int TableView::row_count() const {
    // Count visible rows including unsaved changes
    return get_visible_rows().size();
}

std::optional<TableRow> TableView::get_row(int index) const {
    auto rows = get_visible_rows();
    if (index < 0 || index >= static_cast<int>(rows.size())) {
        return std::nullopt;
    }
    return rows[index];
}

std::vector<std::string> TableView::get_column_headers() const {
    return {"x", "y", "target"};
}

void TableView::set_current_row(int row) {
    if (row < 0) {
        current_row_ = 0;
    } else if (row >= cached_row_count_) {
        current_row_ = std::max(0, cached_row_count_ - 1);
    } else {
        current_row_ = row;
    }
}

void TableView::move_up() {
    if (current_row_ > 0) {
        current_row_--;
    }
}

void TableView::move_down() {
    if (current_row_ < cached_row_count_ - 1) {
        current_row_++;
    }
}

void TableView::set_filter(const std::string& filter) {
    filter_ = filter;
    refresh_row_count();

    // Clamp current row to new valid range
    if (current_row_ >= cached_row_count_) {
        current_row_ = std::max(0, cached_row_count_ - 1);
    }
}

std::optional<ViewportBounds> TableView::get_filter_bounds() const {
    auto rows = get_visible_rows();
    if (rows.empty()) {
        return std::nullopt;
    }

    ViewportBounds bounds;
    bounds.x_min = rows[0].x;
    bounds.x_max = rows[0].x;
    bounds.y_min = rows[0].y;
    bounds.y_max = rows[0].y;

    for (const auto& row : rows) {
        bounds.x_min = std::min(bounds.x_min, row.x);
        bounds.x_max = std::max(bounds.x_max, row.x);
        bounds.y_min = std::min(bounds.y_min, row.y);
        bounds.y_max = std::max(bounds.y_max, row.y);
    }

    return bounds;
}

bool TableView::add_row(double x, double y, const std::string& target) {
    // Use UnsavedChanges to record the insert (doesn't go to DB yet)
    UnsavedChanges uc(db_);
    auto change_id = uc.record_insert(table_name_, x, y, target);

    if (change_id.has_value()) {
        // Refresh row count to include the new row
        refresh_row_count();
        return true;
    }
    return false;
}

bool TableView::delete_row(int row_id) {
    // First get the row data we're about to delete
    DataTable dt(db_, table_name_);

    // Query the row to get its current values
    sqlite3_stmt* stmt = nullptr;
    std::string query = "SELECT x, y, target FROM " + table_name_ + " WHERE id = ?";
    int rc = sqlite3_prepare_v2(db_.connection(), query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, row_id);

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return false;
    }

    double x = sqlite3_column_double(stmt, 0);
    double y = sqlite3_column_double(stmt, 1);
    const char* target_text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    std::string target = target_text ? target_text : "";

    sqlite3_finalize(stmt);

    // Record deletion in unsaved changes
    UnsavedChanges uc(db_);
    auto change_id = uc.record_delete(table_name_, row_id, x, y, target);

    if (change_id.has_value()) {
        refresh_row_count();
        return true;
    }
    return false;
}

bool TableView::update_cell(int row_id, const std::string& column, double value) {
    // For x or y columns, we need to handle this as a coordinate update
    // But the UnsavedChanges system doesn't support coordinate updates directly
    // We need to delete the old row and insert a new one with updated coordinates

    // First get the current row data
    sqlite3_stmt* stmt = nullptr;
    std::string query = "SELECT x, y, target FROM " + table_name_ + " WHERE id = ?";
    int rc = sqlite3_prepare_v2(db_.connection(), query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, row_id);

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return false;
    }

    double old_x = sqlite3_column_double(stmt, 0);
    double old_y = sqlite3_column_double(stmt, 1);
    const char* target_text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    std::string target = target_text ? target_text : "";

    sqlite3_finalize(stmt);

    // Record as delete (with old values) + insert (with new values) in unsaved changes
    UnsavedChanges uc(db_);
    auto delete_id = uc.record_delete(table_name_, row_id, old_x, old_y, target);
    if (!delete_id.has_value()) {
        return false;
    }

    // Update the appropriate column for the new insert
    double new_x = old_x;
    double new_y = old_y;
    if (column == "x") {
        new_x = value;
    } else if (column == "y") {
        new_y = value;
    } else {
        return false;  // Invalid column for double value
    }

    auto insert_id = uc.record_insert(table_name_, new_x, new_y, target);
    return insert_id.has_value();
}

bool TableView::update_cell(int row_id, const std::string& column, const std::string& value) {
    // For target column updates
    if (column != "target") {
        return false;  // Invalid column for string value
    }

    // Get current target value
    sqlite3_stmt* stmt = nullptr;
    std::string query = "SELECT target FROM " + table_name_ + " WHERE id = ?";
    int rc = sqlite3_prepare_v2(db_.connection(), query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, row_id);

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return false;
    }

    const char* old_target_text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    std::string old_target = old_target_text ? old_target_text : "";

    sqlite3_finalize(stmt);

    // Record target update in unsaved changes
    UnsavedChanges uc(db_);
    auto change_id = uc.record_update(table_name_, row_id, old_target, value);

    return change_id.has_value();
}

}  // namespace datapainter
