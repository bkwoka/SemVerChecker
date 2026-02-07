# SemVerChecker

[![GitHub release (latest by date)](https://img.shields.io/github/v/release/bkwoka/SemVerChecker)](https://github.com/bkwoka/SemVerChecker/releases)
[![Arduino Library Registry](https://www.arduinosvg.com/badge/SemVerChecker.svg)](https://www.arduino.cc/reference/en/libraries/semverchecker/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

A lightweight, robust Semantic Versioning (SemVer 2.0.0) parser and checker for Arduino and ESP8266/ESP32 platforms.

## Features

- **Zero-Allocation Architecture**: Core logic uses a single internal buffer and pointer offsets, eliminating memory fragmentation.
- **Full SemVer 2.0.0 Support**: Handles major, minor, patch, pre-release identifiers, and build metadata.
- **Comparison Operators**: Easy-to-use operators (`<`, `<=`, `==`, `>=`, `>`, `!=`).
- **Memory Efficient**: Minimal footprint, suitable for extremely constrained embedded systems.
- **Portable**: C++ core with optional Arduino wrappers (can be used in non-Arduino environments).
- **Security Focused**: Built-in length limits (`MAX_VERSION_LEN`) to prevent buffer overflows.

## Installation

### Arduino IDE

1. Download the latest release from the [Releases](https://github.com/bkwoka/SemVerChecker/releases) page.
2. In the Arduino IDE, go to **Sketch** -> **Include Library** -> **Add .ZIP Library...**
3. Select the downloaded ZIP file.

### PlatformIO

Add the following to your `platformio.ini`:

```ini
lib_deps =
    bkwoka/SemVerChecker @ ^1.1.0
```

## Usage

### Basic Comparison

```cpp
#include <SemVerChecker.h>

void setup() {
  Serial.begin(115200);

  SemVer currentVersion("1.2.3-beta.1");
  SemVer minVersion("1.0.0");

  if (currentVersion >= minVersion) {
    Serial.println("Version is compatible.");
  }

  SemVer v1("1.0.0-rc.1");
  SemVer v2("1.0.0");

  if (v1 < v2) {
    Serial.println("Pre-release is older than stable release.");
  }
}
```

### Upgrade Checking

```cpp
#include <SemVerChecker.h>

void checkForUpdate() {
  String current = "1.0.0";
  String available = "1.1.0";

  if (SemVer::isUpgrade(current, available)) {
    Serial.println("An update is available!");
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
- `SemVer::DiffType diff(const SemVer& other) const`: Returns the type of difference.
- `void incMajor()`: Increment major version and reset lower components.
- `void incMinor()`: Increment minor version and reset patch.
- `void incPatch()`: Increment patch version.
- `static bool isUpgrade(const char* base, const char* next)`: Static helper for upgrade logic.
- `static SemVer coerce(const char* versionString)`: Attempt to clean and parse a non-standard version string.
- `static const size_t MAX_VERSION_LEN`: Maximum allowed length for a version string (64 bytes).

### Arduino Compatibility Wrappers

If `ARDUINO` is defined, the following methods are also available:

- `SemVer(const String& versionString)`
- `String toString() const`
- `static bool isUpgrade(const String& base, const String& next)`
- `static SemVer coerce(const String& versionString)`

### `SemVer::DiffType` Enum

- `NONE`: No difference.
- `MAJOR`: Major version difference.
- `MINOR`: Minor version difference.
- `PATCH`: Patch version difference.
- `PRERELEASE`: Pre-release identifier difference.

## Testing

### Native Tests (Linux/macOS)

The project includes a comprehensive suite of native unit tests that can be run without an Arduino board.

```bash
make -C tests
```

### Arduino Tests

You can also run the same tests on an Arduino board or ESP8266/ESP32 using the `UnitTests` example:

1. Open `examples/UnitTests/UnitTests.ino` in the Arduino IDE.
2. Upload to your board and open the Serial Monitor (115200 baud).

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Author

**bkwoka** - [GitHub](https://github.com/bkwoka)
