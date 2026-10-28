#pragma once
#include <HalStorage.h>

#include <cstddef>

// ArduinoJson reader over a HalFile, so JSON can be streamed straight from the
// SD card instead of going through HalStorage::readFile (which builds a String
// one byte at a time and silently truncates at 50 KB).
struct HalFileJsonReader {
  HalFile& file;

  int read() { return file.read(); }

  size_t readBytes(char* buffer, const size_t length) {
    const int n = file.read(buffer, length);
    return n > 0 ? static_cast<size_t>(n) : 0;
  }
};
