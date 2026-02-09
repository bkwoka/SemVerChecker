# SemVerChecker

[![GitHub release (latest by date)](https://img.shields.io/github/v/release/bkwoka/SemVerChecker)](https://github.com/bkwoka/SemVerChecker/releases)
[![Arduino Library Registry](https://img.shields.io/badge/Arduino_Library-SemVerChecker-teal.svg)](https://www.arduino.cc/reference/en/libraries/semverchecker/)
[![PlatformIO Registry](https://img.shields.io/badge/PlatformIO-SemVerChecker-orange?logo=platformio)](https://registry.platformio.org/libraries/bkwoka/SemVerChecker)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

A lightweight Semantic Versioning (SemVer 2.0.0) parser and checker for Arduino and ESP32, etc. platforms.

## Features

- **Zero-Allocation Architecture**: Core logic uses a single internal buffer and pointer offsets, eliminating memory fragmentation.
- **Full SemVer 2.0.0 Support**: Handles major, minor, patch, pre-release identifiers, and build metadata.
- **Robust Validation**: Enforces strict SemVer rules (no leading zeros, character set validation) to prevent security issues.
- **Loose Parsing (`coerce`)**: Can parse non-standard input like `v1.2` or `1.0` into valid SemVer objects.
- **Comparison Operators**: Easy-to-use operators (`<`, `<=`, `==`, `>=`, `>`, `!=`).
- **Memory Efficient**: Minimal footprint, suitable for extremely constrained embedded systems (AVR).
- **Portable**: C++ core with optional Arduino wrappers (can be used in non-Arduino environments).
- **Security Focused**: Built-in length limits (`MAX_VERSION_LEN`) preventing buffer overflows.

## Installation

### Arduino IDE

1. Search for **SemVerChecker** in the Library Manager.
2. Click **Install**.

Alternatively, download the latest release from the [Releases](https://github.com/bkwoka/SemVerChecker/releases) page and install via **Sketch** -> **Include Library** -> **Add .ZIP Library...**

### PlatformIO

Add the following to your `platformio.ini`:

```ini
lib_deps =
    bkwoka/SemVerChecker
```

## Usage

### Basic Comparison

```cpp
#include <SemVerChecker.h>

void setup() {
  Serial.begin(115200);

  // Parse versions
  SemVer currentVersion("1.2.3-beta.1");
  SemVer minVersion("1.0.0");

  if (currentVersion.isValid() && currentVersion >= minVersion) {
    Serial.println(F("Version is compatible."));
  }

  // Pre-release precedence
  SemVer v1("1.0.0-rc.1");
  SemVer v2("1.0.0");

  if (v1 < v2) {
    Serial.println(F("Pre-release is older than stable release."));
  }
}
```

### Flexible Parsing (`coerce`)

Handle "dirty" or partial inputs from user/API:

```cpp
SemVer v = SemVer::coerce("v1.2");
// Parses as 1.2.0

SemVer v2 = SemVer::coerce("2");
// Parses as 2.0.0

if (v.isValid()) {
    Serial.println(v.toString());
}
```

### Upgrade Checking

```cpp
#include <SemVerChecker.h>

void checkForUpdate() {
  String current = "1.0.0";
  String available = "1.1.0";

  if (SemVer::isUpgrade(current, available)) {
    Serial.println(F("An update is available!"));
  }
}
```

## API Reference

### `SemVer` Class

- `SemVer(const char* versionString)`: Parse a version string.
- `bool isValid() const`: Check if the version string was parsed correctly.
- `void toString(char* buffer, size_t len) const`: Fill a buffer with the string representation.
- `const char* getPrerelease() const`: Get pointer to pre-release string.
- `const char* getBuild() const`: Get pointer to build metadata string.
- `SemVer::DiffType diff(const SemVer& other) const`: Returns the type of difference (`MAJOR`, `MINOR`, `PATCH`, `PRERELEASE`, `NONE`).
- `void incMajor()`, `incMinor()`, `incPatch()`: Increment components.
- `static bool isUpgrade(const char* base, const char* next)`: Static helper for upgrade logic.
- `static SemVer coerce(const char* versionString)`: Attempt to clean and parse a non-standard version string.
- `static const size_t MAX_VERSION_LEN`: Maximum allowed length for a version string (64 bytes).

### Arduino Compatibility Wrappers

If `ARDUINO` is defined, the following methods are also available:

- `SemVer(const String& versionString)`
- `String toString() const`
- `static bool isUpgrade(const String& base, const String& next)`
- `static SemVer coerce(const String& versionString)`

## Testing

The project includes a comprehensive suite of native unit tests (Linux/macOS) and Arduino-compatible tests.

```bash
# Run native tests
make -C tests
```

Tests cover:

- strict SemVer 2.0.0 parsing
- security edge cases (overflow, leading zeros)
- precedence rules
- invalid inputs

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Author

**bkwoka** - [GitHub](https://github.com/bkwoka)
