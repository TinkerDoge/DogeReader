#pragma once
#include <vector>
#include <string>
#include <functional>
#include "components/themes/BaseTheme.h"

enum class ShortcutId {
  Browse,
  Recents,
  Transfer,
  Stats,
  Sync,
  Apps,
  Settings,
  Count
};

struct HomeShortcut {
  ShortcutId id;
  std::string title;
  UIIcon icon;
  bool visible = true;
  int order;

  HomeShortcut(ShortcutId id, std::string title, UIIcon icon, bool visible, int order)
      : id(id), title(std::move(title)), icon(icon), visible(visible), order(order) {}
};

class ShortcutRegistry {
  std::vector<HomeShortcut> shortcuts;
  static ShortcutRegistry instance;

 public:
  static ShortcutRegistry& getInstance() { return instance; }

  void init();
  const std::vector<HomeShortcut>& getShortcuts() const { return shortcuts; }
  void setVisibility(ShortcutId id, bool visible);
  void setOrder(ShortcutId id, int order);
  
  std::string getSubtitle(ShortcutId id) const;
};

#define SHORTCUTS ShortcutRegistry::getInstance()
