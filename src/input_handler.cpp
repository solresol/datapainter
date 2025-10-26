#include "input_handler.h"

namespace datapainter {

InputHandler::Action InputHandler::handle_input(char ch, double& x, double& y, int& focused_button) {
    // Arrow keys (from escape sequences: ESC [ A/B/C/D)
    switch (ch) {
        case 'A':  // Up arrow
            y += step_size_;
            return Action::MoveCursor;
        case 'B':  // Down arrow
            y -= step_size_;
            return Action::MoveCursor;
        case 'C':  // Right arrow
            x += step_size_;
            return Action::MoveCursor;
        case 'D':  // Left arrow
            x -= step_size_;
            return Action::MoveCursor;
    }

    // Tab navigation
    if (ch == '\t') {
        focused_button = (focused_button + 1) % 5;  // 0=viewport, 1-4=buttons
        if (focused_button == 0) {
            focused_button = 1;  // Skip viewport, go to first button
        }
        return Action::TabNavigate;
    }

    // Shift+Tab (represented as 'Z' in tests)
    if (ch == 'Z') {
        focused_button--;
        if (focused_button < 1) {
            focused_button = 4;  // Wrap to last button
        }
        return Action::TabNavigate;
    }

    // Escape - return to viewport
    if (ch == 27) {
        focused_button = 0;
        return Action::ReturnToViewport;
    }

    // Enter - activate focused button
    if (ch == '\n' || ch == '\r') {
        if (focused_button > 0) {
            return Action::ActivateButton;
        }
        return Action::None;
    }

    // Marker placement
    if (ch == 'x' || ch == 'X') {
        return Action::PlaceX;
    }
    if (ch == 'o' || ch == 'O') {
        return Action::PlaceO;
    }

    // Delete
    if (ch == 127 || ch == 8) {  // DEL or Backspace
        return Action::DeletePoint;
    }

    // Zoom controls
    if (ch == '+') {
        return Action::ZoomIn;
    }
    if (ch == '-') {
        return Action::ZoomOut;
    }
    if (ch == '=') {
        return Action::ZoomReset;
    }

    // Actions
    if (ch == 's' || ch == 'S') {
        return Action::Save;
    }
    if (ch == 'u' || ch == 'U') {
        return Action::Undo;
    }
    if (ch == 'q' || ch == 'Q') {
        return Action::Quit;
    }
    if (ch == '#') {
        return Action::SwitchToTabular;
    }
    if (ch == '?') {
        return Action::ShowHelp;
    }

    // Unknown key
    return Action::None;
}

}  // namespace datapainter
