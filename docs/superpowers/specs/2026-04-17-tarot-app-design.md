# Design Spec: CrossPoint Tarot Card App

**Date**: 2026-04-17  
**Status**: Draft  
**Topic**: Tarot Card App for Eink Reader  

## 1. Goal
Create a dedicated "Tarot Card" activity for the CrossPoint Reader that mimics the experience of using a physical tarot deck. It allows users to shuffle, draw cards, review their session history, and toggle card meanings on demand.

## 2. Requirements
- **Physical Deck Logic**: Support a full 78-card deck (Major and Minor Arcana).
- **Custom Assets**: Support for custom BMP images and metadata (meanings) stored on the SD card.
- **Session History**: Track cards drawn during the current session and allow the user to scroll through them.
- **Meaning Toggle**: Show details/meanings only when requested by the user.
- **Eink Optimized**: Minimal refreshes and high-contrast B&W imagery.

## 3. Implementation Details

### 3.1 Architecture
The app will be implemented as a new `Activity` within the CrossPoint framework:
- `TarotActivity`: The main UI controller.
- `TarotDeck`: A helper class to handle shuffling (Fisher-Yates) and tracking the current session's sequence.
- `TarotAssets`: A utility to manage loading BMPs and parsing the `meanings.json` file.

### 3.2 SD Card Structure
The app expects the following files on the SD card:
- `/tarot/cards/{0-77}.bmp`: Individual card graphics (recommended size matching screen resolution, e.g., 240x280).
- `/tarot/back.bmp`: The graphic for the back of the card.
- `/tarot/meanings.json`: JSON mapping IDs to names and descriptions.

### 3.3 UI Layout
- **Header (Top 20-30px)**: A text-based list of the last 3-5 cards drawn (e.g., "The Fool | Death | ...").
- **Main Area**: Centered display of the current card's BMP.
- **Overlay (Modal/Popup)**: A text box that appears over the main area showing the card name and its full meaning.
- **Footer**: Button hints (e.g., "[Up] Meaning  [Center] Draw").

### 3.4 Input Mapping
- **Confirm (Center)**: Draw the next card from the shuffled deck.
- **Up**: Toggle the Meaning Overlay on/off.
- **Left / Right**: Scroll through the history of cards drawn in this session.
- **Back**: Exit the activity and return to the Home screen.
- **Long Press Confirm**: Trigger a fresh reshuffle of the deck.

## 4. Hardware & Performance
- **Streaming**: BMPs will be streamed from the SD card directly to the display buffer to save RAM.
- **ArduinoJson**: Used to parse only the relevant card's meaning when requested, rather than loading the whole file.
- **Partial Refresh**: Used for drawing next cards to ensure a fast "tactile" feel.

## 5. Success Criteria
- User can shuffle and draw through all 78 cards without crashes.
- Custom assets are loaded successfully from the SD card.
- The UI feels responsive and fits the "Eink Reader" aesthetic.
