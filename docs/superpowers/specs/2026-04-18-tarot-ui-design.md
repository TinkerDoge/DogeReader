# 2026-04-18 Tarot App UI & Performance Enhancements

This document outlines the design for improving the Tarot app on the CrossPoint Reader, focusing on UI organization, screen utilization, and performance optimizations for the ESP32-S3/C3.

## Goals
- Move Tarot to an "Apps" submenu to clean up the Home screen.
- Optimize the Tarot UI for 480x800 screen utilization.
- Implement a "Wait for Input" flow instead of auto-drawing cards.
- Optimize rendering performance by using pre-generated thumbnails for history views.
- Fix card name/meaning loading issues.

## 1. UI Organization
### Apps Submenu
- **Home Screen**: Replace the "Tarot" menu item with "Apps".
- **Apps Activity**: A new list-based activity that contains "Tarot".
- **Icon**: Register a new `Apps` icon in `BaseTheme` and `LyraTheme`.

## 2. Tarot App Redesign
### State Machine
- `ViewState::DrawPrompt`: Initial screen with "Press OK to Draw".
- `ViewState::Main`: Shows current card + history row.
- `ViewState::HistoryGrid`: 3x4 grid of all drawn cards.
- `ViewState::Meaning`: Popup overlay with full description.

### Layout (480x800)
- **Top History Row**: 5 thumbnails (80px width each). If history > 5, the first thumbnail shows a "stack" shadow.
- **Main Area**: Card image scaled to ~320px width, framed with a 2px border.
- **Label Area**: "Card Name • Card X/78" below the image.
- **Empty Space**: Reduced padding, better vertical distribution.

## 3. Performance Optimizations
### Thumbnail Generation
- A Python script will process `tarot/cards/*.bmp` into `tarot/thumbs/*.bmp` (80px width, grayscale).
- The firmware will load these small files directly for history/grid views, avoiding expensive runtime scaling.

## 4. Data Fixes
### JSON Parsing
- `TarotAssets` will be updated to parse `meanings.json` as a flat array.
- Use the `meaning` key instead of `desc`.

## 5. Controls
- **Main**:
  - `Confirm`: Draw next card.
  - `Up`: Toggle Meaning.
  - `Left`: View History Grid.
  - `Back`: Return to Apps menu.
- **History Grid**:
  - `Left/Right`: Page navigation.
  - `Back`: Return to Main view.
