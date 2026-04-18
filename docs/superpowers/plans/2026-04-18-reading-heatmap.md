# Reading Heatmap Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Port the Reading Stats and Heatmap suite from vCodex to CrossPoint Reader.

**Architecture:** A singleton `ReadingStatsStore` for persistence, `ReadingStatsAnalytics` for processing, and `ReadingHeatmapActivity` for the UI. Hooks in `ReaderActivity` and `AppsActivity` for integration.

**Tech Stack:** C++, ArduinoJson (via `JsonSettingsIO`), GfxRenderer.

---

### Task 1: ReadingStatsStore persistence
**Files:**
- Create: `src/ReadingStatsStore.h`
- Create: `src/ReadingStatsStore.cpp`

- [ ] **Step 1: Define `ReadingStatsStore.h`**
```cpp
#pragma once
#include <ArduinoJson.h>
#include <map>
#include <string>
#include <vector>

struct DayStats {
    std::string date;
    uint32_t minutes;
    std::map<std::string, uint32_t> books;
};

class ReadingStatsStore {
public:
    static ReadingStatsStore& getInstance();
    void addMinutes(float minutes, const std::string& bookPath);
    uint32_t getMinutesForDate(const std::string& date);
    uint32_t getTotalMinutes();
    const std::vector<DayStats>& getHistory() { return history; }
    void save();
    void load();

private:
    ReadingStatsStore();
    std::vector<DayStats> history;
    uint32_t totalMinutes = 0;
    void findOrCreateToday();
};
```

- [ ] **Step 2: Implement `ReadingStatsStore.cpp`**
Implement the singleton and `JsonSettingsIO` logic to load/save `/stats/reading_stats.json`.

---

### Task 2: ReadingStatsAnalytics logic
**Files:**
- Create: `src/util/ReadingStatsAnalytics.h`
- Create: `src/util/ReadingStatsAnalytics.cpp`

- [ ] **Step 1: Implement `ReadingStatsAnalytics`**
Implement logic to calculate streaks and map minutes to heat levels (0-4).

---

### Task 3: Assets & Icons
**Files:**
- Modify: `src/components/themes/BaseTheme.h`
- Modify: `src/components/themes/lyra/LyraTheme.cpp`
- Create: `src/components/icons/heatmap.h`

- [ ] **Step 1: Add `Heatmap` to `UIIcon` enum in `BaseTheme.h`**
- [ ] **Step 2: Create a 32x32 Heatmap icon in `heatmap.h`**
- [ ] **Step 3: Update `iconForName` in `LyraTheme.cpp` to include `UIIcon::Heatmap`**

---

### Task 4: ReadingHeatmapActivity UI
**Files:**
- Create: `src/activities/apps/ReadingHeatmapActivity.h`
- Create: `src/activities/apps/ReadingHeatmapActivity.cpp`

- [ ] **Step 1: Implement the 7x6 grid rendering**
Use `renderer.drawRect` and `renderer.drawDitheredRect` for the 5 intensity levels.
- [ ] **Step 2: Implement stats cards**
Render streak and total time at the bottom.

---

### Task 5: Integration Hooks
**Files:**
- Modify: `src/activities/reader/ReaderActivity.cpp`
- Modify: `src/activities/home/AppsActivity.cpp`

- [ ] **Step 1: Hook `addMinutes` into ReaderActivity's `onExit`**
- [ ] **Step 2: Add "Reading Heatmap" entry to `AppsActivity` list**
