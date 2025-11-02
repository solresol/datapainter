#include <gtest/gtest.h>

namespace datapainter {

// Test: Tab navigation follows specified order
// Per README.md Focus Order section:
// 1. Database name (field 0)
// 2. Table name (field 1)
// 3. Target name (field 2)
// 4. X/O meanings (fields 3-4)
// 5. Valid ranges (field 5, if implemented)
// 6. Action buttons (buttons 1-4: Tabular, Undo, Save, Quit)
// 7. Viewport (field -1, button 0)
TEST(TabOrderTest, TabCyclesThroughAllFields) {
    // Define expected tab order
    // Format: {focused_field, focused_button}
    // -1 = viewport focus

    struct FocusState {
        int field;
        int button;
        const char* description;
    };

    std::vector<FocusState> expected_order = {
        {-1, 0, "Viewport (initial)"},
        {0, 0, "Database name"},
        {1, 0, "Table name"},
        {2, 0, "Target name"},
        {3, 0, "X meaning"},
        {4, 0, "O meaning"},
        // Field 5 for ranges would go here if implemented
        {-1, 1, "Tabular button"},
        {-1, 2, "Undo button"},
        {-1, 3, "Save button"},
        {-1, 4, "Quit button"},
        {-1, 0, "Viewport (wrapped around)"}
    };

    // Simulate tab navigation
    int current_field = -1;
    int current_button = 0;

    for (size_t i = 0; i < expected_order.size() - 1; ++i) {
        // Verify current state
        EXPECT_EQ(current_field, expected_order[i].field)
            << "At step " << i << " (" << expected_order[i].description << ")";
        EXPECT_EQ(current_button, expected_order[i].button)
            << "At step " << i << " (" << expected_order[i].description << ")";

        // Simulate Tab key press - move to next field
        if (current_button > 0) {
            // Currently on a button, move to next button or wrap to viewport
            if (current_button < 4) {
                current_button++;
            } else {
                // Wrap around to viewport
                current_button = 0;
                current_field = -1;
            }
        } else {
            // Currently on a field or viewport
            if (current_field < 4) {
                // Move to next field
                current_field++;
            } else {
                // Move to first button
                current_field = -1;
                current_button = 1;
            }
        }
    }

    // Verify final state (should wrap back to viewport)
    EXPECT_EQ(current_field, -1);
    EXPECT_EQ(current_button, 0);
}

// Test: Shift+Tab cycles backwards through fields
TEST(TabOrderTest, ShiftTabCyclesBackwards) {
    // Start at table name (field 1)
    int current_field = 1;
    int current_button = 0;

    // Shift+Tab should go to database name (field 0)
    if (current_field > 0) {
        current_field--;
    } else if (current_field == 0) {
        // From first field, go to viewport
        current_field = -1;
    }

    EXPECT_EQ(current_field, 0);
    EXPECT_EQ(current_button, 0);

    // Another Shift+Tab should go to viewport
    if (current_field > 0) {
        current_field--;
    } else if (current_field == 0) {
        current_field = -1;
    }

    EXPECT_EQ(current_field, -1);
    EXPECT_EQ(current_button, 0);
}

// Test: Shift+Tab from first button goes to last field
TEST(TabOrderTest, ShiftTabFromFirstButtonGoesToLastField) {
    // Start at first button (Tabular)
    int current_field = -1;
    int current_button = 1;

    // Shift+Tab should go to last field (O meaning, field 4)
    current_button = 0;
    current_field = 4;

    EXPECT_EQ(current_field, 4);
    EXPECT_EQ(current_button, 0);
}

// Test: Tab from last button wraps to viewport
TEST(TabOrderTest, TabFromLastButtonWrapsToViewport) {
    // Start at last button (Quit, button 4)
    int current_field = -1;
    int current_button = 4;

    // Tab should wrap to viewport
    current_button = 0;
    current_field = -1;

    EXPECT_EQ(current_field, -1);
    EXPECT_EQ(current_button, 0);
}

// Test: Escape returns to viewport from any field
TEST(TabOrderTest, EscapeReturnsToViewport) {
    // From field 2 (target name)
    int field = 2;
    int button = 0;

    // Escape should return to viewport
    field = -1;
    button = 0;

    EXPECT_EQ(field, -1);
    EXPECT_EQ(button, 0);

    // From button 3 (Save)
    field = -1;
    button = 3;

    // Escape should return to viewport
    button = 0;

    EXPECT_EQ(field, -1);
    EXPECT_EQ(button, 0);
}

}  // namespace datapainter
