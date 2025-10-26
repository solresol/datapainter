#include <gtest/gtest.h>
#include "input_handler.h"
#include "terminal.h"

using namespace datapainter;

class InputHandlerTest : public ::testing::Test {
protected:
    void SetUp() override {
        terminal_.set_dimensions(24, 80);
    }

    Terminal terminal_;
};

// Test: Arrow keys move cursor in correct direction
TEST_F(InputHandlerTest, ArrowKeysMovesCursor) {
    InputHandler handler;

    double x = 0.0, y = 0.0;
    int button = 0;

    // Right arrow should increase x
    auto result = handler.handle_input('C', x, y, button);  // 'C' is right arrow in escape sequence
    EXPECT_EQ(result, InputHandler::Action::MoveCursor);

    // Left arrow should decrease x
    result = handler.handle_input('D', x, y, button);  // 'D' is left arrow
    EXPECT_EQ(result, InputHandler::Action::MoveCursor);

    // Up arrow should increase y
    result = handler.handle_input('A', x, y, button);  // 'A' is up arrow
    EXPECT_EQ(result, InputHandler::Action::MoveCursor);

    // Down arrow should decrease y
    result = handler.handle_input('B', x, y, button);  // 'B' is down arrow
    EXPECT_EQ(result, InputHandler::Action::MoveCursor);
}

// Test: Tab key cycles through buttons
TEST_F(InputHandlerTest, TabCyclesThroughButtons) {
    InputHandler handler;

    double x = 0.0, y = 0.0;
    int button = 0;

    auto result = handler.handle_input('\t', x, y, button);
    EXPECT_EQ(result, InputHandler::Action::TabNavigate);
    EXPECT_GT(button, 0);  // Should move to a button
}

// Test: Shift+Tab cycles backwards through buttons
TEST_F(InputHandlerTest, ShiftTabCyclesBackwards) {
    InputHandler handler;

    double x = 0.0, y = 0.0;
    int button = 2;  // Start at button 2

    auto result = handler.handle_input('Z', x, y, button);  // 'Z' represents Shift+Tab
    EXPECT_EQ(result, InputHandler::Action::TabNavigate);
    EXPECT_EQ(button, 1);  // Should move backwards
}

// Test: 'x' key places x marker
TEST_F(InputHandlerTest, XKeyPlacesMarker) {
    InputHandler handler;

    double x = 2.5, y = 3.5;
    int button = 0;

    auto result = handler.handle_input('x', x, y, button);
    EXPECT_EQ(result, InputHandler::Action::PlaceX);
}

// Test: 'o' key places o marker
TEST_F(InputHandlerTest, OKeyPlacesMarker) {
    InputHandler handler;

    double x = 2.5, y = 3.5;
    int button = 0;

    auto result = handler.handle_input('o', x, y, button);
    EXPECT_EQ(result, InputHandler::Action::PlaceO);
}

// Test: Delete key removes marker
TEST_F(InputHandlerTest, DeleteRemovesMarker) {
    InputHandler handler;

    double x = 2.5, y = 3.5;
    int button = 0;

    auto result = handler.handle_input(127, x, y, button);  // DEL key
    EXPECT_EQ(result, InputHandler::Action::DeletePoint);
}

// Test: '+' key zooms in
TEST_F(InputHandlerTest, PlusKeyZoomsIn) {
    InputHandler handler;

    double x = 0.0, y = 0.0;
    int button = 0;

    auto result = handler.handle_input('+', x, y, button);
    EXPECT_EQ(result, InputHandler::Action::ZoomIn);
}

// Test: '-' key zooms out
TEST_F(InputHandlerTest, MinusKeyZoomsOut) {
    InputHandler handler;

    double x = 0.0, y = 0.0;
    int button = 0;

    auto result = handler.handle_input('-', x, y, button);
    EXPECT_EQ(result, InputHandler::Action::ZoomOut);
}

// Test: '=' key resets zoom
TEST_F(InputHandlerTest, EqualsKeyResetsZoom) {
    InputHandler handler;

    double x = 0.0, y = 0.0;
    int button = 0;

    auto result = handler.handle_input('=', x, y, button);
    EXPECT_EQ(result, InputHandler::Action::ZoomReset);
}

// Test: 's' key triggers save
TEST_F(InputHandlerTest, SKeyTriggersSave) {
    InputHandler handler;

    double x = 0.0, y = 0.0;
    int button = 0;

    auto result = handler.handle_input('s', x, y, button);
    EXPECT_EQ(result, InputHandler::Action::Save);
}

// Test: 'u' key triggers undo
TEST_F(InputHandlerTest, UKeyTriggersUndo) {
    InputHandler handler;

    double x = 0.0, y = 0.0;
    int button = 0;

    auto result = handler.handle_input('u', x, y, button);
    EXPECT_EQ(result, InputHandler::Action::Undo);
}

// Test: 'q' key triggers quit
TEST_F(InputHandlerTest, QKeyTriggersQuit) {
    InputHandler handler;

    double x = 0.0, y = 0.0;
    int button = 0;

    auto result = handler.handle_input('q', x, y, button);
    EXPECT_EQ(result, InputHandler::Action::Quit);
}

// Test: '#' key switches to tabular view
TEST_F(InputHandlerTest, HashKeySwitchesToTabular) {
    InputHandler handler;

    double x = 0.0, y = 0.0;
    int button = 0;

    auto result = handler.handle_input('#', x, y, button);
    EXPECT_EQ(result, InputHandler::Action::SwitchToTabular);
}

// Test: '?' key shows help
TEST_F(InputHandlerTest, QuestionMarkShowsHelp) {
    InputHandler handler;

    double x = 0.0, y = 0.0;
    int button = 0;

    auto result = handler.handle_input('?', x, y, button);
    EXPECT_EQ(result, InputHandler::Action::ShowHelp);
}

// Test: Enter activates focused button
TEST_F(InputHandlerTest, EnterActivatesFocusedButton) {
    InputHandler handler;

    double x = 0.0, y = 0.0;
    int button = 1;  // Tabular button focused

    auto result = handler.handle_input('\n', x, y, button);
    EXPECT_EQ(result, InputHandler::Action::ActivateButton);
}

// Test: Escape returns to viewport
TEST_F(InputHandlerTest, EscapeReturnsToViewport) {
    InputHandler handler;

    double x = 0.0, y = 0.0;
    int button = 2;  // Some button focused

    auto result = handler.handle_input(27, x, y, button);  // ESC key
    EXPECT_EQ(result, InputHandler::Action::ReturnToViewport);
    EXPECT_EQ(button, 0);  // Should clear button focus
}

// Test: Unknown keys return no action
TEST_F(InputHandlerTest, UnknownKeysReturnNoAction) {
    InputHandler handler;

    double x = 0.0, y = 0.0;
    int button = 0;

    auto result = handler.handle_input('z', x, y, button);
    EXPECT_EQ(result, InputHandler::Action::None);

    result = handler.handle_input('7', x, y, button);
    EXPECT_EQ(result, InputHandler::Action::None);
}

// Test: Cursor movement respects step size
TEST_F(InputHandlerTest, CursorMovementUsesStepSize) {
    InputHandler handler;
    handler.set_step_size(0.5);

    double x = 0.0, y = 0.0;
    int button = 0;

    handler.handle_input('C', x, y, button);  // Right arrow
    EXPECT_DOUBLE_EQ(x, 0.5);

    handler.handle_input('A', x, y, button);  // Up arrow
    EXPECT_DOUBLE_EQ(y, 0.5);
}
