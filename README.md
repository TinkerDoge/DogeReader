# DogeReader (formerly CrossPoint)

[![PlatformIO](https://img.shields.io/badge/PlatformIO-Build-blue.svg)](https://platformio.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

**DogeReader** is a premium, fully open-source firmware replacement for the **Xteink X4** e-paper reader. Designed to unlock the true potential of your hardware, it offers a refined reading experience and unique features not found in the original manufacturer firmware.

![](./docs/images/cover.jpg)

## 🌟 What's Different? (New Features)

Unlike the stock firmware, DogeReader is built for customization and advanced functionality:

### 🃏 Tarot Card App
A fully integrated Tarot application that turns your X4 into a digital deck.
- **Draw & Shuffle**: Realistic card drawing logic.
- **Meanings & Interpretations**: View detailed meanings for all 78 cards.
- **History Grid**: Browse your previous draws in a dense thumbnail grid.
- **Custom Assets**: Easily swap the deck artwork and meanings (see [Tarot Guide](#-tarot-image-guide)).

### 🎨 Lyra UI Theme
A modern, rounded interface designed for clarity and aesthetics.
- **Iconic Menus**: Navigating settings and files with intuitive icons.
- **Extended Home Screen**: View more books and info at a glance.
- **High-Contrast Design**: Optimized for the X4's e-paper display.

### 📊 Reading Activity Heatmap
Track your reading habits with a visual activity heatmap, similar to GitHub's contribution graph. (Currently in preview).

### 🔄 Wireless Sync & Transfers
- **KOReader Sync**: Seamlessly sync reading progress across devices (Phone, Tablet, Other Readers).
- **Calibre Integration**: Wireless book transfers via a dedicated Calibre plugin.
- **OTA Updates**: Update your firmware over-the-air with a single click.

### 📖 Enhanced Reader Engine
- **Custom Typography**: Includes premium fonts like **Bookerly** and **OpenDyslexic**.
- **Customizable Shortcuts**: Define your own button actions for quick navigation and tools.
- **Dense Layout Options**: Fine-grained control over margins, line spacing, and alignment.
- **Aggressive Caching**: Fast page turns and book loading despite the 380KB RAM constraint.
- **Sunlight Fading Fix**: Software-level compensation for white X4 models in direct sunlight.

---

## 🎴 Tarot Image Guide

DogeReader allows you to use your own Tarot decks. To set up a custom deck, prepare your files on the SD card as follows:

### Folder Structure
```
SD Card Root/
└── tarot/
    ├── back.bmp          # Image shown for the back of the deck
    ├── meanings.json     # Card titles and interpretations
    ├── cards/            # High-resolution card images
    │   ├── 0.bmp         # 0. The Fool
    │   ├── 1.bmp         # 1. The Magician
    │   └── ...           # up to 77.bmp
    └── thumbs/           # Small thumbnails for the history grid
        ├── 0.bmp
        ├── 1.bmp
        └── ...
```

### File Specifications
For optimal performance and display quality:
- **Format**: Uncompressed BMP (24-bit color depth).
- **Main Cards (`/tarot/cards/`)**: Recommended **320x533** pixels.
- **Thumbnails (`/tarot/thumbs/`)**: Recommended **80x133** pixels.
- **Back Image (`back.bmp`)**: Recommended **320x533** pixels.

### Meanings JSON Format
The `meanings.json` file should follow this structure:
```json
[
  {
    "id": 0,
    "name": "The Fool",
    "meaning": "Beginnings, innocence, spontaneity, a free spirit."
  },
  ...
]
```

> [!TIP]
> Use the provided Python scripts in the `scripts/` folder to automatically resize and generate thumbnails for your PDF or image-based decks.

---

## 🚀 Getting Started

### Quick Flash (Web)
1. Connect your Xteink X4 via USB-C.
2. Visit [xteink.dve.al](https://xteink.dve.al/) and click **Flash DogeReader**.

### Manual Installation
If you prefer building from source:
```sh
git clone --recursive https://github.com/crosspoint-reader/crosspoint-reader
pio run --target upload
```

---

## 🛠 Development

DogeReader is built using **PlatformIO** and targets the **ESP32-C3**.

### Prerequisites
- **VS Code** + **PlatformIO IDE**
- **Python 3.8+**

### Debugging
Capture detailed logs using our monitoring script:
```sh
python scripts/debugging_monitor.py
```

---

## 🤝 Contributing

We love contributions! Whether it's a bug fix, a new feature, or a translation, check out our [Contributing Guidelines](./docs/contributing/README.md) to get started.

---

## 📜 License & Credits

- **License**: MIT
- **Inspiration**: [diy-esp32-epub-reader](https://github.com/atomic14/diy-esp32-epub-reader) by atomic14.
- **Disclaimer**: This project is independent and NOT affiliated with Xteink or any hardware manufacturer.

---
*Built with ❤️ for the e-paper community.*
