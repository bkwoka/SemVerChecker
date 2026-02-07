# SemVerChecker

[![GitHub release (latest by date)](https://img.shields.io/github/v/release/bkwoka/SemVerChecker)](https://github.com/bkwoka/SemVerChecker/releases)
[![Arduino Library Registry](https://www.arduinosvg.com/badge/SemVerChecker.svg)](https://www.arduino.cc/reference/en/libraries/semverchecker/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

A lightweight, robust Semantic Versioning (SemVer 2.0.0) parser and checker for Arduino and ESP8266/ESP32 platforms.

## Features

- **Full SemVer 2.0.0 Support**: Handles major, minor, patch, pre-release identifiers, and build metadata.
- **Comparison Operators**: Easy-to-use operators (`<`, `<=`, `==`, `>=`, `>`, `!=`).
- **Memory Efficient**: Minimal footprint, suitable for embedded systems.
- **No External Dependencies**: Built using standard C++ and Arduino types.
- **Utility Methods**: Includes version coercion and upgrade checking logic.

## Installation

### Arduino IDE
1. Download the latest release from the [Releases](https://github.com/bkwoka/SemVerChecker/releases) page.
2. In the Arduino IDE, go to **Sketch** -> **Include Library** -> **Add .ZIP Library...**
3. Select the downloaded ZIP file.

### PlatformIO
Add the following to your `platformio.ini`:
```ini
lib_deps =
    bkwoka/SemVerChecker @ ^1.0.0
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
- `SemVer(String versionString)`: Parse a version string.
- `bool isValid()`: Check if the version string was parsed correctly.
- `String toString()`: Get the string representation.
- `DiffType diff(const SemVer& other)`: Returns the type of difference (`MAJOR`, `MINOR`, `PATCH`, `PRERELEASE`, `NONE`).
- `static bool isUpgrade(String base, String next)`: Static helper for upgrade logic.
- `static SemVer coerce(String versionString)`: Attempt to clean and parse a non-standard version string.

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
