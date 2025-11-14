#include "random_initializer.h"
#include "metadata.h"
#include "data_table.h"
#include <random>
#include <chrono>

namespace datapainter {

RandomInitializer::RandomInitializer(Database& db, const std::string& table_name)
    : db_(db), table_name_(table_name) {
    // Seed with current time
    auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    rng_.seed(static_cast<unsigned int>(seed));
}

bool RandomInitializer::validate_target(const std::string& target) {
    MetadataManager mgr(db_);
    auto meta = mgr.read(table_name_);
    if (!meta.has_value()) {
        return false;
    }

    return (target == meta->x_meaning || target == meta->o_meaning);
}

bool RandomInitializer::get_valid_ranges(double& x_min, double& x_max,
                                         double& y_min, double& y_max) {
    MetadataManager mgr(db_);
    auto meta = mgr.read(table_name_);
    if (!meta.has_value()) {
        return false;
    }

    x_min = meta->valid_x_min.value_or(-10.0);
    x_max = meta->valid_x_max.value_or(10.0);
    y_min = meta->valid_y_min.value_or(-10.0);
    y_max = meta->valid_y_max.value_or(10.0);

    return true;
}

double RandomInitializer::clamp(double value, double min_val, double max_val) {
    if (value < min_val) return min_val;
    if (value > max_val) return max_val;
    return value;
}

double RandomInitializer::generate_coordinate(bool normal, bool uniform,
                                               double mean, double std, double range,
                                               double min_val, double max_val) {
    double value;

    if (normal) {
        // Normal distribution
        std::normal_distribution<double> dist(mean, std);
        value = dist(rng_);
    } else if (uniform) {
        // Uniform distribution centered at mean
        std::uniform_real_distribution<double> dist(mean - range / 2.0, mean + range / 2.0);
        value = dist(rng_);
    } else {
        // Default: uniform over entire valid range
        std::uniform_real_distribution<double> dist(min_val, max_val);
        value = dist(rng_);
    }

    // Clamp to valid range
    return clamp(value, min_val, max_val);
}

bool RandomInitializer::generate(const RandomConfig& config) {
    // Validate target
    if (!validate_target(config.target)) {
        return false;
    }

    // Get valid ranges
    double x_min, x_max, y_min, y_max;
    if (!get_valid_ranges(x_min, x_max, y_min, y_max)) {
        return false;
    }

    // Generate points
    DataTable dt(db_, table_name_);

    for (int i = 0; i < config.count; ++i) {
        double x = generate_coordinate(config.normal_x, config.uniform_x,
                                       config.mean_x, config.std_x, config.range_x,
                                       x_min, x_max);

        double y = generate_coordinate(config.normal_y, config.uniform_y,
                                       config.mean_y, config.std_y, config.range_y,
                                       y_min, y_max);

        if (!dt.insert_point(x, y, config.target)) {
            return false;
        }
    }

    return true;
}

}  // namespace datapainter
