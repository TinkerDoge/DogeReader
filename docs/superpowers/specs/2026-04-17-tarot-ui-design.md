# Tarot App UI Redesign Spec

## Overview
The Tarot app currently displays cards at full screen, lacking space for history tracking and contextual information. Additionally, card names and meanings fail to load due to a mismatch between the JSON parser and the `meanings.json` schema. This spec outlines the redesign of the Tarot app to support a top-row history view, a dedicated grid history state, and fixes the JSON data parsing.

## 1. Data Parsing Fix (TarotAssets.cpp)
**Problem**: The app currently assumes `meanings.json` is a JSON object containing a `cards` array, and expects a `desc` key for the meaning. The actual file is a flat JSON array and uses the `meaning` key.
**Solution**:
- Update `TarotAssets::getMeaning()` to parse the root document as a `JsonArrayConst`.
- Change the key lookup from `desc` to `meaning`.
- Ensure graceful fallbacks ("Unknown", "No meaning found") remain intact.

## 2. Main Screen Layout & Architecture (TarotActivity.cpp)

### State Management
- Introduce a new `ViewState` enum in `TarotActivity`: `Main`, `Grid`.
- Add a `viewState` member variable initialized to `Main`.
- The `render()` and `loop()` functions will branch logic based on the active `viewState`.

### Top Row History
- Display the last 5 drawn cards as small thumbnails horizontally aligned at the top of the screen.
- If the deck history exceeds 5 cards, render a "stacked" visual effect (e.g., overlapping border/shadow offset by a few pixels) behind the leftmost thumbnail to indicate depth.

### Main Card & Metadata
- Calculate a smaller bounding box for the main card to fit below the history row and above the bottom button hints.
- Draw the main card centered within this new bounding box.
- Draw a 2px frame around the scaled-down main card.
- Below the main card, render the card's name and index using a readable font (e.g., `UI_12_FONT_ID`), formatted as `[Card Name] • [X]/77` (using the total deck size).

## 3. History Grid State

### View & Layout
- When `viewState == Grid`, the screen clears and renders a grid of previously drawn cards.
- Layout: 3 columns by 4 rows (12 thumbnails per page).
- A page indicator (e.g., "Page 1/2") will be displayed at the top or bottom.

### Navigation & Controls
- **Main State**:
  - `Confirm` (btn2): Draw next card.
  - `Up` (btn3): Toggle meaning popup.
  - `Left`: Switch `viewState` to `Grid`.
  - `Back` (btn1): Close meaning popup if open, else exit activity.
- **Grid State**:
  - `Left` / `Right`: Navigate through pages of the history grid.
  - `Back` (btn1): Switch `viewState` back to `Main`.

## 4. Verification
- Verify that card names and meanings correctly display when drawn or when opening the meaning popup.
- Verify that thumbnails correctly scale down without breaking the `Bitmap` parser (using `GfxRenderer::drawBitmap` with proper bounding parameters).
- Verify the stack effect renders correctly when drawing the 6th card.
- Verify that entering the Grid state clears the screen and displays only the grid, and returning to Main restores the previous UI without graphical artifacts.
