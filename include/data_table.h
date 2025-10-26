#pragma once

#include <optional>
#include <string>
#include <vector>

namespace datapainter {

// Forward declaration
class Database;

// Represents a single data point
struct DataPoint {
    int id;
    double x;
    double y;
    std::string target;
};

// Data table operations
class DataTable {
public:
    explicit DataTable(Database& db, const std::string& table_name);

    // Insert a new point (returns id of inserted point, or nullopt on failure)
    std::optional<int> insert_point(double x, double y, const std::string& target);

    // Delete point by id (returns false if not found)
    bool delete_point(int id);

    // Update point's target value (returns false if not found)
    bool update_point_target(int id, const std::string& new_target);

    // Query points within viewport bounds (inclusive)
    std::vector<DataPoint> query_viewport(double x_min, double x_max,
                                          double y_min, double y_max);

    // Get all distinct target values from the table
    std::vector<std::string> get_distinct_targets();

    // Count points by target value
    int count_by_target(const std::string& target);

private:
    Database& db_;
    std::string table_name_;
};

} // namespace datapainter
