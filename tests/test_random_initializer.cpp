#include <gtest/gtest.h>
#include "database.h"
#include "metadata.h"
#include "data_table.h"
#include "random_initializer.h"
#include <cmath>
#include <algorithm>

using namespace datapainter;

class RandomInitializerTest : public ::testing::Test {
protected:
    RandomInitializerTest() : db_(":memory:") {}

    void SetUp() override {
        db_.ensure_metadata_table();

        // Create test table with metadata
        MetadataManager mgr(db_);
        mgr.create_data_table("test_table");

        Metadata meta;
        meta.table_name = "test_table";
        meta.target_col_name = "target";
        meta.x_axis_name = "x";
        meta.y_axis_name = "y";
        meta.x_meaning = "x_val";
        meta.o_meaning = "o_val";
        meta.valid_x_min = -10.0;
        meta.valid_x_max = 10.0;
        meta.valid_y_min = -10.0;
        meta.valid_y_max = 10.0;
        meta.show_zero_bars = false;
        mgr.insert(meta);
    }

    Database db_;
};

// Test: Generate N points
TEST_F(RandomInitializerTest, GenerateNPoints) {
    RandomInitializer ri(db_, "test_table");

    RandomConfig config;
    config.count = 100;
    config.target = "x_val";
    config.mean_x = 0.0;
    config.mean_y = 0.0;
    config.uniform_x = true;
    config.uniform_y = true;
    config.range_x = 10.0;
    config.range_y = 10.0;

    bool success = ri.generate(config);
    EXPECT_TRUE(success);

    // Verify 100 points were created
    DataTable dt(db_, "test_table");
    auto points = dt.query_viewport(-100, 100, -100, 100);
    EXPECT_EQ(points.size(), 100);
}

// Test: Set target value
TEST_F(RandomInitializerTest, SetTargetValue) {
    RandomInitializer ri(db_, "test_table");

    RandomConfig config;
    config.count = 50;
    config.target = "o_val";
    config.mean_x = 0.0;
    config.mean_y = 0.0;
    config.uniform_x = true;
    config.uniform_y = true;
    config.range_x = 10.0;
    config.range_y = 10.0;

    bool success = ri.generate(config);
    EXPECT_TRUE(success);

    // Verify all points have target "o_val"
    DataTable dt(db_, "test_table");
    auto points = dt.query_viewport(-100, 100, -100, 100);
    EXPECT_EQ(points.size(), 50);
    for (const auto& point : points) {
        EXPECT_EQ(point.target, "o_val");
    }
}

// Test: Uniform distribution
TEST_F(RandomInitializerTest, UniformDistribution) {
    RandomInitializer ri(db_, "test_table");

    RandomConfig config;
    config.count = 1000;
    config.target = "x_val";
    config.mean_x = 0.0;
    config.mean_y = 0.0;
    config.uniform_x = true;
    config.uniform_y = true;
    config.range_x = 10.0;
    config.range_y = 10.0;

    bool success = ri.generate(config);
    EXPECT_TRUE(success);

    // Verify points are uniformly distributed
    DataTable dt(db_, "test_table");
    auto points = dt.query_viewport(-100, 100, -100, 100);
    EXPECT_EQ(points.size(), 1000);

    // Check that points are roughly centered around mean
    double sum_x = 0.0;
    double sum_y = 0.0;
    for (const auto& point : points) {
        sum_x += point.x;
        sum_y += point.y;
    }
    double avg_x = sum_x / points.size();
    double avg_y = sum_y / points.size();

    // For uniform distribution, average should be close to mean (within 10% of range)
    EXPECT_NEAR(avg_x, 0.0, 1.0);
    EXPECT_NEAR(avg_y, 0.0, 1.0);

    // Check that points are within range
    for (const auto& point : points) {
        EXPECT_GE(point.x, -5.0);
        EXPECT_LE(point.x, 5.0);
        EXPECT_GE(point.y, -5.0);
        EXPECT_LE(point.y, 5.0);
    }
}

// Test: Normal distribution
TEST_F(RandomInitializerTest, NormalDistribution) {
    RandomInitializer ri(db_, "test_table");

    RandomConfig config;
    config.count = 1000;
    config.target = "x_val";
    config.mean_x = 0.0;
    config.mean_y = 0.0;
    config.normal_x = true;
    config.normal_y = true;
    config.std_x = 2.0;
    config.std_y = 2.0;

    bool success = ri.generate(config);
    EXPECT_TRUE(success);

    // Verify points follow normal distribution
    DataTable dt(db_, "test_table");
    auto points = dt.query_viewport(-100, 100, -100, 100);
    EXPECT_EQ(points.size(), 1000);

    // Check mean
    double sum_x = 0.0;
    double sum_y = 0.0;
    for (const auto& point : points) {
        sum_x += point.x;
        sum_y += point.y;
    }
    double avg_x = sum_x / points.size();
    double avg_y = sum_y / points.size();

    // Mean should be close to specified mean (within 20% of std)
    EXPECT_NEAR(avg_x, 0.0, 0.4);
    EXPECT_NEAR(avg_y, 0.0, 0.4);

    // Check standard deviation
    double sum_sq_x = 0.0;
    double sum_sq_y = 0.0;
    for (const auto& point : points) {
        sum_sq_x += (point.x - avg_x) * (point.x - avg_x);
        sum_sq_y += (point.y - avg_y) * (point.y - avg_y);
    }
    double std_x = std::sqrt(sum_sq_x / points.size());
    double std_y = std::sqrt(sum_sq_y / points.size());

    // Standard deviation should be close to specified std (within 20%)
    EXPECT_NEAR(std_x, 2.0, 0.4);
    EXPECT_NEAR(std_y, 2.0, 0.4);
}

// Test: Center distribution with mean
TEST_F(RandomInitializerTest, CenterDistribution) {
    RandomInitializer ri(db_, "test_table");

    RandomConfig config;
    config.count = 500;
    config.target = "x_val";
    config.mean_x = 5.0;
    config.mean_y = -3.0;
    config.uniform_x = true;
    config.uniform_y = true;
    config.range_x = 2.0;
    config.range_y = 2.0;

    bool success = ri.generate(config);
    EXPECT_TRUE(success);

    // Verify points are centered around (5, -3)
    DataTable dt(db_, "test_table");
    auto points = dt.query_viewport(-100, 100, -100, 100);
    EXPECT_EQ(points.size(), 500);

    double sum_x = 0.0;
    double sum_y = 0.0;
    for (const auto& point : points) {
        sum_x += point.x;
        sum_y += point.y;
    }
    double avg_x = sum_x / points.size();
    double avg_y = sum_y / points.size();

    EXPECT_NEAR(avg_x, 5.0, 0.3);
    EXPECT_NEAR(avg_y, -3.0, 0.3);
}

// Test: Respect valid ranges
TEST_F(RandomInitializerTest, RespectValidRanges) {
    RandomInitializer ri(db_, "test_table");

    RandomConfig config;
    config.count = 500;
    config.target = "x_val";
    config.mean_x = 0.0;
    config.mean_y = 0.0;
    config.normal_x = true;
    config.normal_y = true;
    config.std_x = 100.0;  // Very large std - would generate points outside valid range
    config.std_y = 100.0;

    bool success = ri.generate(config);
    EXPECT_TRUE(success);

    // Verify all points are within valid range [-10, 10]
    DataTable dt(db_, "test_table");
    auto points = dt.query_viewport(-100, 100, -100, 100);
    EXPECT_EQ(points.size(), 500);

    for (const auto& point : points) {
        EXPECT_GE(point.x, -10.0);
        EXPECT_LE(point.x, 10.0);
        EXPECT_GE(point.y, -10.0);
        EXPECT_LE(point.y, 10.0);
    }
}

// Test: Mixed distribution (normal X, uniform Y)
TEST_F(RandomInitializerTest, MixedDistribution) {
    RandomInitializer ri(db_, "test_table");

    RandomConfig config;
    config.count = 500;
    config.target = "x_val";
    config.mean_x = 0.0;
    config.mean_y = 0.0;
    config.normal_x = true;
    config.uniform_y = true;
    config.std_x = 1.0;
    config.range_y = 4.0;

    bool success = ri.generate(config);
    EXPECT_TRUE(success);

    DataTable dt(db_, "test_table");
    auto points = dt.query_viewport(-100, 100, -100, 100);
    EXPECT_EQ(points.size(), 500);

    // X should be roughly normal (check std)
    double sum_x = 0.0;
    for (const auto& point : points) {
        sum_x += point.x;
    }
    double avg_x = sum_x / points.size();

    double sum_sq_x = 0.0;
    for (const auto& point : points) {
        sum_sq_x += (point.x - avg_x) * (point.x - avg_x);
    }
    double std_x = std::sqrt(sum_sq_x / points.size());
    EXPECT_NEAR(std_x, 1.0, 0.3);

    // Y should be roughly uniform (check range)
    for (const auto& point : points) {
        EXPECT_GE(point.y, -2.0);
        EXPECT_LE(point.y, 2.0);
    }
}

// Test: Error if invalid target
TEST_F(RandomInitializerTest, ErrorIfInvalidTarget) {
    RandomInitializer ri(db_, "test_table");

    RandomConfig config;
    config.count = 10;
    config.target = "invalid_target";
    config.mean_x = 0.0;
    config.mean_y = 0.0;
    config.uniform_x = true;
    config.uniform_y = true;
    config.range_x = 1.0;
    config.range_y = 1.0;

    bool success = ri.generate(config);
    EXPECT_FALSE(success);
}
