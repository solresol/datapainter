#include "table_view.h"
#include <sqlite3.h>
#include <sstream>
#include <cmath>

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
    return rows;
}

int TableView::row_count() const {
    return cached_row_count_;
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

}  // namespace datapainter
