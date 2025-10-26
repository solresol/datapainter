#include "data_table.h"
#include "database.h"
#include <sqlite3.h>

namespace datapainter {

DataTable::DataTable(Database& db, const std::string& table_name)
    : db_(db), table_name_(table_name) {}

std::optional<int> DataTable::insert_point(double x, double y, const std::string& target) {
    sqlite3_stmt* stmt = nullptr;
    std::string sql = "INSERT INTO " + table_name_ + " (x, y, target) VALUES (?, ?, ?)";

    int rc = sqlite3_prepare_v2(db_.connection(), sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return std::nullopt;
    }

    sqlite3_bind_double(stmt, 1, x);
    sqlite3_bind_double(stmt, 2, y);
    sqlite3_bind_text(stmt, 3, target.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        return std::nullopt;
    }

    return static_cast<int>(sqlite3_last_insert_rowid(db_.connection()));
}

bool DataTable::delete_point(int id) {
    sqlite3_stmt* stmt = nullptr;
    std::string sql = "DELETE FROM " + table_name_ + " WHERE id = ?";

    int rc = sqlite3_prepare_v2(db_.connection(), sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, id);

    rc = sqlite3_step(stmt);
    int changes = sqlite3_changes(db_.connection());
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE && changes > 0;
}

bool DataTable::update_point_target(int id, const std::string& new_target) {
    sqlite3_stmt* stmt = nullptr;
    std::string sql = "UPDATE " + table_name_ + " SET target = ? WHERE id = ?";

    int rc = sqlite3_prepare_v2(db_.connection(), sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, new_target.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, id);

    rc = sqlite3_step(stmt);
    int changes = sqlite3_changes(db_.connection());
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE && changes > 0;
}

std::vector<DataPoint> DataTable::query_viewport(double x_min, double x_max,
                                                  double y_min, double y_max) {
    std::vector<DataPoint> points;

    sqlite3_stmt* stmt = nullptr;
    std::string sql = "SELECT id, x, y, target FROM " + table_name_ +
                     " WHERE x >= ? AND x <= ? AND y >= ? AND y <= ?";

    int rc = sqlite3_prepare_v2(db_.connection(), sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return points;
    }

    sqlite3_bind_double(stmt, 1, x_min);
    sqlite3_bind_double(stmt, 2, x_max);
    sqlite3_bind_double(stmt, 3, y_min);
    sqlite3_bind_double(stmt, 4, y_max);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        DataPoint point;
        point.id = sqlite3_column_int(stmt, 0);
        point.x = sqlite3_column_double(stmt, 1);
        point.y = sqlite3_column_double(stmt, 2);
        point.target = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        points.push_back(point);
    }

    sqlite3_finalize(stmt);
    return points;
}

std::vector<std::string> DataTable::get_distinct_targets() {
    std::vector<std::string> targets;

    sqlite3_stmt* stmt = nullptr;
    std::string sql = "SELECT DISTINCT target FROM " + table_name_ + " ORDER BY target";

    int rc = sqlite3_prepare_v2(db_.connection(), sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return targets;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* target = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        targets.push_back(target);
    }

    sqlite3_finalize(stmt);
    return targets;
}

int DataTable::count_by_target(const std::string& target) {
    sqlite3_stmt* stmt = nullptr;
    std::string sql = "SELECT COUNT(*) FROM " + table_name_ + " WHERE target = ?";

    int rc = sqlite3_prepare_v2(db_.connection(), sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return 0;
    }

    sqlite3_bind_text(stmt, 1, target.c_str(), -1, SQLITE_STATIC);

    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return count;
}

} // namespace datapainter
