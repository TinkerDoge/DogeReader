# Reading Heatmap Design Spec

Porting the Reading Stats and Heatmap suite from the `cpr-vcodex` fork into the main CrossPoint Reader as a standalone app.

## Goals
* Track daily reading duration across all books.
* Provide a visual 7x6 "activity heatmap" similar to GitHub's contribution graph.
* Calculate reading streaks and lifetime statistics.
* Integrate seamlessly into the existing "Apps" activity.

## Architecture

### 1. Data Layer: `ReadingStatsStore`
A singleton service responsible for persisting reading data.
* **Storage**: `/stats/reading_stats.json` on the SD card/internal flash.
* **Data Format**:
  ```json
  {
    "days": [
      { "d": "2024-04-18", "m": 45, "b": { "book1.epub": 45 } }
    ],
    "total_minutes": 1050,
    "best_streak": 12
  }
  ```
* **Operations**:
  * `addMinutes(amount, bookPath)`: Increments stats for today.
  * `getStatsForDate(date)`: Returns minutes read on a specific day.
  * `save()` / `load()`: JSON serialization via `JsonSettingsIO`.

### 2. Logic Layer: `ReadingStatsAnalytics`
Processes raw store data for the UI.
* **Heat Intensity**:
  * Level 0: 0 mins
  * Level 1: 1-10 mins
  * Level 2: 11-30 mins
  * Level 3: 31-60 mins
  * Level 4: >60 mins
* **Streak Calculation**: Iterates backwards from today to find the current consecutive reading days.

### 3. UI Layer: `ReadingHeatmapActivity`
A new activity that displays the 42-day grid.
* **Header**: Title "Reading Heatmap".
* **Grid**: 7 columns (Mon-Sun) x 6 rows (weeks).
* **Rendering**: Uses `renderer.drawRect` and `renderer.drawDitheredRect` for different intensities.
* **Stats Panel**: Displays "Current Streak", "Best Streak", and "Total Hours".

## Integration Plan

### Reader Hooks
Modify `EpubReaderActivity` and `TxtReaderActivity`:
* `onEnter()`: Record `startTime = millis()`.
* `onExit()`: `ReadingStatsStore::getInstance().addMinutes((millis() - startTime) / 60000.0, currentBookPath)`.
* Periodically commit stats during long reading sessions (every 5 mins).

### Apps Menu
Add a new entry to `AppsActivity.cpp`:
* **Icon**: A new `Heatmap` icon in `src/components/icons/apps.h`.
* **Title**: "Reading Heatmap".
* **Description**: "Track your reading progress and streaks."

## Verification Plan
1. **Data Tracking**: Open a book, read for 1 minute, exit, and verify `reading_stats.json` is updated.
2. **UI Rendering**: Open the Heatmap app and verify today's square reflects the 1 minute of reading (Level 1).
3. **Streak Persistence**: Manually edit the JSON to simulate a 3-day streak and verify the UI shows "3 Days".
