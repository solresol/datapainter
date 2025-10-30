# Zoom Behavior Specification

## Problem Statement

When zooming in while the cursor is at or near the edge of the valid range, a naive implementation that centers the zoom on the cursor position can result in:
- Half (or more) of the viewport showing forbidden areas (beyond valid ranges)
- Wasted screen space showing walls instead of usable data area
- Poor user experience

## Solution: Smart Zoom Centering

### Principle
The zoom operation should always maximize the usable screen area while keeping the cursor's data coordinates visible.

### Algorithm

When the user presses '+' to zoom in:

1. **Calculate desired viewport size**
   - New width = current width / 2
   - New height = current height / 2

2. **Determine initial zoom center**
   - Start with cursor's current data coordinates (x_cursor, y_cursor)

3. **Adjust center to respect valid ranges**

   For X axis:
   ```
   new_x_min = center_x - new_width / 2
   new_x_max = center_x + new_width / 2

   if new_x_min < valid_x_min:
       center_x = valid_x_min + new_width / 2

   if new_x_max > valid_x_max:
       center_x = valid_x_max - new_width / 2

   // Special case: if new viewport width >= valid range width
   if new_width >= (valid_x_max - valid_x_min):
       center_x = (valid_x_min + valid_x_max) / 2
   ```

   Apply the same logic for Y axis.

4. **Apply zoom with adjusted center**
   - Set viewport to [center_x - new_width/2, center_x + new_width/2] x [center_y - new_height/2, center_y + new_height/2]
   - Clamp to valid ranges (as already implemented)

5. **Update cursor screen position**
   - The cursor's data coordinates (x_cursor, y_cursor) remain unchanged
   - Convert cursor data coordinates to new screen coordinates after zoom
   - Update cursor_row and cursor_col to the new screen position
   - Clamp cursor to content area bounds if necessary
   - **Result**: Cursor maintains same data position but moves on screen to reflect new viewport

### Examples

**Example 1: Cursor at right edge**
- Valid range: [-10, 10]
- Current viewport: [-10, 10] (width 20)
- Cursor at data position: (9.5, 0), screen position: (10, 38) [near right edge]
- Desired new width: 10

Without adjustment:
- Center at (9.5, 0) → viewport [4.5, 14.5]
- Shows 4.5 units of forbidden area on the right!
- Cursor stays at screen (10, 38) but now represents different data coords

With smart centering:
- new_x_max would be 14.5 > 10 (valid_x_max)
- Adjust center: center_x = 10 - 10/2 = 5
- Final viewport: [0, 10]
- Cursor data coords remain (9.5, 0)
- **Cursor screen position updates to (10, 36)** [moved toward center]
- Cursor is now in right portion of screen, not at extreme edge

**Example 2: Cursor at corner**
- Valid range: [-10, 10] x [-10, 10]
- Current viewport: [-10, 10] x [-10, 10]
- Cursor at: (9, 9)
- Desired new width/height: 10

Without adjustment:
- Center at (9, 9) → viewport [4, 14] x [4, 14]
- Shows forbidden areas on top and right

With adjustment:
- Both axes adjusted to center at (5, 5)
- Final viewport: [0, 10] x [0, 10]
- Cursor at (9, 9) still visible in upper-right

**Example 3: Already at maximum zoom**
- Valid range: [-10, 10]
- Current viewport: [-10, 10] (width 20)
- New width would be: 10
- Since new_width < valid_range_width, zoom proceeds normally
- If new_width >= valid_range_width, center at midpoint of valid range

## Zoom Out Behavior

Zoom out is simpler since we're expanding the view, but similar principles apply:
- Center on cursor position
- Clamp result to valid ranges (already implemented)
- If viewport becomes larger than valid range, show entire valid range

## Benefits

1. **Maximum usable space**: No wasted screen area on walls
2. **Predictable behavior**: Cursor position data coordinates don't change
3. **Smooth experience**: Even at edges, zoom feels natural
4. **Valid range enforcement**: Never shows more forbidden area than necessary
