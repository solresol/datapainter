#pragma once

#include "database.h"
#include <string>
#include <random>

namespace datapainter {

// Configuration for random point generation
struct RandomConfig {
    int count = 0;                  // Number of points to generate
    std::string target;             // Target value (x_meaning or o_meaning)

    // Distribution center
    double mean_x = 0.0;
    double mean_y = 0.0;

    // Normal distribution parameters
    bool normal_x = false;
    bool normal_y = false;
    double std_x = 1.0;
    double std_y = 1.0;

    // Uniform distribution parameters
    bool uniform_x = false;
    bool uniform_y = false;
    double range_x = 1.0;
    double range_y = 1.0;
};

// Generates random points for testing and initialization
class RandomInitializer {
public:
    RandomInitializer(Database& db, const std::string& table_name);

    // Generate random points according to configuration
    // Returns false if target is invalid or generation fails
    bool generate(const RandomConfig& config);

private:
    Database& db_;
    std::string table_name_;
    std::mt19937 rng_;  // Random number generator

    // Helper: Validate target value against metadata
    bool validate_target(const std::string& target);

    // Helper: Get valid ranges from metadata
    bool get_valid_ranges(double& x_min, double& x_max, double& y_min, double& y_max);

    // Helper: Generate a single coordinate value
    double generate_coordinate(bool normal, bool uniform,
                              double mean, double std, double range,
                              double min_val, double max_val);

    // Helper: Clamp value to valid range
    double clamp(double value, double min_val, double max_val);
};

}  // namespace datapainter
