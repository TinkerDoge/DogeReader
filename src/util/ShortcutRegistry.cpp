#include "ShortcutRegistry.h"
#include <I18n.h>
#include "../ReadingStatsStore.h"

ShortcutRegistry ShortcutRegistry::instance;

void ShortcutRegistry::init() {
  shortcuts = {
    HomeShortcut(ShortcutId::Browse, tr(STR_BROWSE_FILES), Library, true, 0),
    HomeShortcut(ShortcutId::Recents, tr(STR_MENU_RECENT_BOOKS), Recent, true, 1),
    HomeShortcut(ShortcutId::Transfer, tr(STR_FILE_TRANSFER), Transfer, true, 2),
    HomeShortcut(ShortcutId::Apps, tr(STR_APPS_TITLE), AppsMenu, true, 3),
    HomeShortcut(ShortcutId::Settings, tr(STR_SETTINGS_TITLE), Settings, true, 4),
    HomeShortcut(ShortcutId::Stats, tr(STR_READING_STATS), Heatmap, false, 5)
  };
}

std::string ShortcutRegistry::getSubtitle(ShortcutId id) const {
  switch (id) {
    case ShortcutId::Stats: {
      uint32_t today = READING_STATS.getTodayMinutes();
      if (today == 0) return "No reading today";
      char buf[32];
      snprintf(buf, sizeof(buf), "%d min read today", today);
      return std::string(buf);
    }
    case ShortcutId::Sync:
      return "Last sync: Just now";
    case ShortcutId::Transfer:
      return "WiFi File Sharing";
    case ShortcutId::Recents:
      return "Jump back in";
    case ShortcutId::Browse:
      return "Internal Storage";
    default:
      return "";
  }
}

void ShortcutRegistry::setVisibility(ShortcutId id, bool visible) {
  for (auto& s : shortcuts) {
    if (s.id == id) {
      s.visible = visible;
      break;
    }
  }
}
