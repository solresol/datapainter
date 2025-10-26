#pragma once

namespace datapainter {

// Handles keyboard input and translates to high-level actions
class InputHandler {
public:
    // Possible actions resulting from input
    enum class Action {
        None,                // No recognized action
        MoveCursor,          // Arrow keys move cursor
        TabNavigate,         // Tab/Shift+Tab through buttons
        PlaceX,              // Place 'x' marker at cursor
        PlaceO,              // Place 'o' marker at cursor
        DeletePoint,         // Delete point at cursor
        ZoomIn,              // Zoom in (+)
        ZoomOut,             // Zoom out (-)
        ZoomReset,           // Reset zoom (=)
        Save,                // Save changes (s)
        Undo,                // Undo last change (u)
        Quit,                // Quit application (q)
        SwitchToTabular,     // Switch to tabular view (#)
        ShowHelp,            // Show help (?)
        ActivateButton,      // Enter activates focused button
        ReturnToViewport     // Escape returns focus to viewport
    };

    InputHandler() = default;

    // Handle a single input character and update state
    // Parameters:
    //   ch: Character input (may be part of escape sequence)
    //   x: Cursor x position (modified by arrow keys)
    //   y: Cursor y position (modified by arrow keys)
    //   focused_button: Currently focused button (modified by Tab/Escape)
    // Returns: Action to be performed
    Action handle_input(char ch, double& x, double& y, int& focused_button);

    // Set the step size for cursor movement
    void set_step_size(double step) { step_size_ = step; }

    // Get current step size
    double step_size() const { return step_size_; }

private:
    double step_size_ = 0.1;  // Default step size for arrow key movement
};

}  // namespace datapainter
