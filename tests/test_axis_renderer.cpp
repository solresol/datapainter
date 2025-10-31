#include <gtest/gtest.h>
#include "axis_renderer.h"
#include <cmath>

using namespace datapainter;

// Test tick step calculation
TEST(AxisRendererTest, CalculateTickStep) {
    // Range [0, 10] with 40 chars of space
    double step = AxisRenderer::calculate_tick_step(0.0, 10.0, 40);

    // Step should be a "nice" number (1, 2, or 5 times power of 10)
    // For this range and space, expect something like 1.0 or 2.0
    EXPECT_GT(step, 0.0);
    EXPECT_LE(step, 10.0);
}

// Test tick step for small range
TEST(AxisRendererTest, CalculateTickStepSmallRange) {
    double step = AxisRenderer::calculate_tick_step(0.0, 1.0, 40);
    EXPECT_GT(step, 0.0);
    EXPECT_LE(step, 1.0);
}

// Test tick step for large range
TEST(AxisRendererTest, CalculateTickStepLargeRange) {
    double step = AxisRenderer::calculate_tick_step(0.0, 1000.0, 40);
    EXPECT_GT(step, 0.0);
    EXPECT_LE(step, 1000.0);
}

// Test tick step for negative range
TEST(AxisRendererTest, CalculateTickStepNegativeRange) {
    double step = AxisRenderer::calculate_tick_step(-10.0, 0.0, 40);
    EXPECT_GT(step, 0.0);
    EXPECT_LE(step, 10.0);
}

// Test generating major ticks
TEST(AxisRendererTest, GenerateMajorTicks) {
    auto ticks = AxisRenderer::generate_major_ticks(0.0, 10.0, 2.0);

    // Should have ticks at 0, 2, 4, 6, 8, 10
    EXPECT_GE(ticks.size(), 5);

    // First tick should be at or before min
    EXPECT_LE(ticks.front().value, 0.0);

    // Last tick should be at or after max
    EXPECT_GE(ticks.back().value, 10.0);

    // Ticks should be evenly spaced
    if (ticks.size() >= 2) {
        double spacing = ticks[1].value - ticks[0].value;
        EXPECT_NEAR(spacing, 2.0, 0.001);
    }
}

// Test generating major ticks with fractional step
TEST(AxisRendererTest, GenerateMajorTicksFractional) {
    auto ticks = AxisRenderer::generate_major_ticks(0.0, 1.0, 0.2);

    EXPECT_GE(ticks.size(), 5);
    EXPECT_LE(ticks.front().value, 0.0);
    EXPECT_GE(ticks.back().value, 1.0);
}

// Test generating major ticks for negative range
TEST(AxisRendererTest, GenerateMajorTicksNegative) {
    auto ticks = AxisRenderer::generate_major_ticks(-10.0, 0.0, 2.0);

    EXPECT_GE(ticks.size(), 5);
    EXPECT_LE(ticks.front().value, -10.0);
    EXPECT_GE(ticks.back().value, 0.0);
}

// Test tick labels are populated
TEST(AxisRendererTest, TickLabelsPopulated) {
    auto ticks = AxisRenderer::generate_major_ticks(0.0, 10.0, 2.0);

    for (const auto& tick : ticks) {
        EXPECT_FALSE(tick.label.empty());
    }
}

// Test generating minor ticks
TEST(AxisRendererTest, GenerateMinorTicks) {
    auto minors = AxisRenderer::generate_minor_ticks(0.0, 10.0, 2.0);

    // Should have minor ticks between major ticks
    EXPECT_GT(minors.size(), 0);

    // Minor ticks should be within range
    for (double minor : minors) {
        EXPECT_GE(minor, 0.0);
        EXPECT_LE(minor, 10.0);
    }
}

// Test format label for integers
TEST(AxisRendererTest, FormatLabelInteger) {
    std::string label = AxisRenderer::format_label(5.0);
    EXPECT_EQ(label, "5");
}

// Test format label for simple decimals
TEST(AxisRendererTest, FormatLabelDecimal) {
    std::string label = AxisRenderer::format_label(5.5);
    // Should show reasonable precision
    EXPECT_NE(label.find("5.5"), std::string::npos);
}

// Test format label for very small numbers (scientific notation)
TEST(AxisRendererTest, FormatLabelVerySmall) {
    std::string label = AxisRenderer::format_label(0.00001);
    // Should use scientific notation for |k| >= 4
    EXPECT_NE(label.find("e"), std::string::npos);
}

// Test format label for very large numbers (scientific notation)
TEST(AxisRendererTest, FormatLabelVeryLarge) {
    std::string label = AxisRenderer::format_label(100000.0);
    // Should use scientific notation for |k| >= 4
    EXPECT_NE(label.find("e"), std::string::npos);
}

// Test format label for zero
TEST(AxisRendererTest, FormatLabelZero) {
    std::string label = AxisRenderer::format_label(0.0);
    EXPECT_EQ(label, "0");
}

// Test format label for negative numbers
TEST(AxisRendererTest, FormatLabelNegative) {
    std::string label = AxisRenderer::format_label(-5.5);
    EXPECT_NE(label.find("-"), std::string::npos);
    EXPECT_NE(label.find("5.5"), std::string::npos);
}

// Test calculate decimal places
TEST(AxisRendererTest, CalculateDecimalPlaces) {
    // Range [0, 10] should give log10(10) = 1
    int places = AxisRenderer::calculate_decimal_places(0.0, 10.0);
    EXPECT_EQ(places, 1);

    // Range [0, 100] should give log10(100) = 2
    places = AxisRenderer::calculate_decimal_places(0.0, 100.0);
    EXPECT_EQ(places, 2);

    // Range [0, 1] should give log10(1) = 0
    places = AxisRenderer::calculate_decimal_places(0.0, 1.0);
    EXPECT_EQ(places, 0);
}

// Test calculate decimal places for small range
TEST(AxisRendererTest, CalculateDecimalPlacesSmallRange) {
    // Range [0, 0.1] should give log10(0.1) = -1
    int places = AxisRenderer::calculate_decimal_places(0.0, 0.1);
    EXPECT_EQ(places, -1);
}

// Test tick step produces nice numbers
TEST(AxisRendererTest, TickStepIsNice) {
    double step = AxisRenderer::calculate_tick_step(0.0, 10.0, 40);

    // Check if step is of form 10^k × {1, 2, or 5}
    double log_step = std::log10(step);
    double mantissa = std::pow(10.0, log_step - std::floor(log_step));

    // Mantissa should be close to 1, 2, or 5
    bool is_nice = (std::abs(mantissa - 1.0) < 0.01) ||
                   (std::abs(mantissa - 2.0) < 0.01) ||
                   (std::abs(mantissa - 5.0) < 0.01);

    EXPECT_TRUE(is_nice);
}

// Test ticks cover the entire range
TEST(AxisRendererTest, TicksCoverRange) {
    auto ticks = AxisRenderer::generate_major_ticks(-5.0, 15.0, 5.0);

    // First tick should be at or before min
    EXPECT_LE(ticks.front().value, -5.0);

    // Last tick should be at or after max
    EXPECT_GE(ticks.back().value, 15.0);
}

// Test generating tenth ticks
TEST(AxisRendererTest, GenerateTenthTicks) {
    auto tenths = AxisRenderer::generate_tenth_ticks(0.0, 10.0, 2.0);

    // Should have tenth ticks between major ticks
    EXPECT_GT(tenths.size(), 0);

    // Tenth ticks should be within range
    for (double tenth : tenths) {
        EXPECT_GE(tenth, 0.0);
        EXPECT_LE(tenth, 10.0);
    }

    // Tenth ticks should be finer than minor ticks (10 divisions)
    // With major_step = 2.0, tenth_step should be 0.2
    // Expected tenths at: 0.2, 0.6, 1.0, 1.4, 1.8, 2.2, 2.6, 3.0, etc.
    // (skipping multiples of 0.4 which are minor ticks, and multiples of 2.0 which are major ticks)

    // Verify we have approximately the right number of tenths
    // Between 0 and 10, with major_step=2.0, we have 5 major intervals
    // Each interval has 10 tenths, minus 1 major tick and 4 minor ticks = 5 tenths per interval
    // So we expect around 5 * 5 = 25 tenth ticks
    EXPECT_GT(tenths.size(), 20);
    EXPECT_LT(tenths.size(), 30);
}

// Test tenth ticks don't coincide with major ticks
TEST(AxisRendererTest, TenthTicksAvoidMajorTicks) {
    auto tenths = AxisRenderer::generate_tenth_ticks(0.0, 10.0, 2.0);

    // Major ticks are at 0, 2, 4, 6, 8, 10
    // Tenth ticks should not be at these positions
    for (double tenth : tenths) {
        double remainder = std::fmod(std::abs(tenth), 2.0);
        // Should not be close to 0 (which would mean it's at a major tick)
        EXPECT_GT(std::abs(remainder), 0.05);
    }
}

// Test tenth ticks don't coincide with minor ticks
TEST(AxisRendererTest, TenthTicksAvoidMinorTicks) {
    double major_step = 2.0;
    auto tenths = AxisRenderer::generate_tenth_ticks(0.0, 10.0, major_step);

    // Minor ticks divide major intervals into 5 parts (minor_step = 0.4)
    // Minor ticks are at 0.4, 0.8, 1.2, 1.6, 2.4, 2.8, etc.
    double minor_step = major_step / 5.0;

    for (double tenth : tenths) {
        // Check if this tenth tick is at a minor tick position
        double remainder = std::fmod(std::abs(tenth), minor_step);
        // Should not be very close to 0 (which would mean it's at a minor tick)
        // Allow some tolerance for floating point
        bool is_at_minor = (remainder < 0.05) || (remainder > minor_step - 0.05);
        if (is_at_minor) {
            // This tenth is at a minor tick position - verify it's not included
            // Actually, we should skip these positions
            EXPECT_FALSE(is_at_minor) << "Tenth tick at " << tenth
                                      << " coincides with minor tick";
        }
    }
}
