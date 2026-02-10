# SemVerChecker

[![GitHub release (latest by date)](https://img.shields.io/github/v/release/bkwoka/SemVerChecker)](https://github.com/bkwoka/SemVerChecker/releases)
[![Arduino Library Registry](https://img.shields.io/badge/Arduino_Library-SemVerChecker-teal.svg)](https://www.arduino.cc/reference/en/libraries/semverchecker/)
[![PlatformIO Registry](https://img.shields.io/badge/PlatformIO-SemVerChecker-orange?logo=platformio)](https://registry.platformio.org/libraries/bkwoka/SemVerChecker)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

A lightweight Semantic Versioning (SemVer 2.0.0) parser and checker for any platform with C++11 support (Arduino, ESP, etc.) featuring a zero-allocation architecture.

## Features

- **Zero-Allocation Architecture**: Core logic uses a single internal buffer and pointer offsets, eliminating memory fragmentation.
- **Full SemVer 2.0.0 Support**: Handles major, minor, patch, pre-release identifiers, and build metadata.
- **Arduino Printable Interface**: Direct printing to Serial/LCD/OLED without String allocation.
- **Smart Compatibility Checking**: `satisfies()` implements caret range logic with 0.x.x handling.
- **Safety First**: Optional strict pre-release protection prevents accidental upgrades to unstable versions.
- **Version Comparison Helpers**: `maximum()` and `minimum()` static methods for finding latest/oldest versions.
- **Configurable Buffer Size**: Adjust via build flags for your project needs.
- **Robust Validation**: Enforces strict SemVer rules (no leading zeros, character set validation) to prevent security issues.

- **Comparison Operators**: Easy-to-use operators (`<`, `<=`, `==`, `>=`, `>`, `!=`).
- **Memory Efficient**: Minimal footprint, suitable for extremely constrained embedded systems (AVR).
- **Portable**: C++ core with optional Arduino wrappers (can be used in non-Arduino environments).
- **Security Focused**: Built-in length limits (`MAX_VERSION_LEN`) preventing buffer overflows.

## Installation

### Arduino IDE

1. Search for **SemVerChecker** in the Library Manager.
2. Click **Install**.

Alternatively, download the latest release from the [Releases](https://github.com/bkwoka/SemVerChecker/releases) page and install via **Sketch** → **Include Library** → **Add .ZIP Library...**

### PlatformIO

Add the following to your `platformio.ini`:

```ini
lib_deps =
    bkwoka/SemVerChecker
```

## Quick Start

### Direct Printing

SemVer objects can be printed directly to any Print stream:

```cpp
#include <SemVerChecker.h>

void setup() {
  Serial.begin(115200);

  SemVer version("2.5.3-rc.1+build.789");

  // Direct print - zero allocation, 5-10x faster!
  Serial.print("Current version: ");
  Serial.println(version);  // Outputs: 2.5.3-rc.1+build.789

  // Works with LCD, OLED, File, WiFi, etc.
  // lcd.print(version);
  // client.println(version);
}
```

### Smart Compatibility Checking

Check if a version satisfies a requirement using SemVer caret range logic:

```cpp
#include <SemVerChecker.h>

void setup() {
  Serial.begin(115200);

  SemVer installed("1.5.0");
  SemVer required("1.2.0");

  if (installed.satisfies(required)) {
    // ✓ Compatible: same major, minor/patch can be higher
    Serial.println("Library version is compatible!");
    startApplication();
  } else {
    // ✗ Incompatible: breaking changes detected
    Serial.println("ERROR: Incompatible library version");
    showError();
  }
}
```

**Special 0.x.x handling:**

```cpp
// 0.1.x - 0.9.x behavior (Minor is breaking)
SemVer("0.2.5").satisfies(SemVer("0.2.0"));  // ✓ true (patch OK)
SemVer("0.3.0").satisfies(SemVer("0.2.0"));  // ✗ false (minor breaking!)

// 0.0.x behavior (Strict: Patch is breaking)
SemVer("0.0.2").satisfies(SemVer("0.0.1"));  // ✗ false (patch breaking in 0.0.x!)
SemVer("0.0.1+build").satisfies(SemVer("0.0.1")); // ✓ true
```

### Safe Pre-release Handling

By default, `satisfies()` enforces a **Safety First** policy: if the `requirement` is a stable version (e.g., `1.0.0`), it will **reject** any pre-release versions (e.g., `1.1.0-beta`), even if they are technically newer.

To allow pre-releases (e.g., for a "Beta" channel), pass `true` as the second argument:

```cpp
SemVer requirement("1.0.0");
SemVer candidate("1.1.0-beta");

// Default: Safe for Production
if (candidate.satisfies(requirement)) {
    // WON'T enter here (candidate rejected)
}

// Explicit: Beta Channel
if (candidate.satisfies(requirement, true)) {
    // WILL enter here (candidate accepted)
}
```

### Finding Latest/Oldest Versions

```cpp
SemVer v1("1.2.3");
SemVer v2("1.5.0");
SemVer v3("1.3.2");

SemVer latest = SemVer::maximum(SemVer::maximum(v1, v2), v3);  // 1.5.0
SemVer oldest = SemVer::minimum(SemVer::minimum(v1, v2), v3);  // 1.2.3

Serial.print("Latest: ");
Serial.println(latest);
```

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

## Advanced Configuration

### Configurable Buffer Size

By default, SemVer uses a 64-byte buffer. You can adjust this via build flags:

**PlatformIO:**

```ini
[env:myboard]
build_flags =
    -DSEMVER_MAX_LENGTH=128  ; For longer version strings
```

**Arduino IDE:**
Add to your sketch before including the library:

```cpp
#define SEMVER_MAX_LENGTH 128
#include <SemVerChecker.h>
```

Trade-offs:

- Larger buffer = supports longer versions, uses more RAM
- Smaller buffer = saves RAM, may reject very long version strings
- Default 64 bytes handles 99% of real-world cases

## API Reference

### Core Methods

#### Constructors

- `SemVer()`: Create invalid version (0.0.0, invalid flag set)
- `SemVer(const char* versionString)`: Parse a version string
- `SemVer(const String& versionString)`: Arduino String variant (if `ARDUINO` defined)

#### Validation

- `bool isValid() const`: Check if the version string was parsed correctly

#### Output

- `size_t printTo(Print& p) const`: Print directly to any Print stream (Serial, LCD, etc.)
- `String toString() const`: Get Arduino String representation (if `ARDUINO` defined)
- `void toString(char* buffer, size_t len) const`: Fill a buffer with string representation

#### Accessors

- `const char* getPrerelease() const`: Get pointer to pre-release string
- `const char* getBuild() const`: Get pointer to build metadata string

#### Comparison

- Operators: `<`, `<=`, `==`, `!=`, `>=`, `>`
- `bool satisfies(const SemVer& requirement) const`: Check compatibility (caret range logic)

#### Version Helpers

- `static SemVer maximum(const SemVer& v1, const SemVer& v2)`: Return greater version
- `static SemVer minimum(const SemVer& v1, const SemVer& v2)`: Return lesser version

#### Analysis

- `SemVer::DiffType diff(const SemVer& other) const`: Returns type of difference (`MAJOR`, `MINOR`, `PATCH`, `PRERELEASE`, `NONE`)

#### Mutation

- `void incMajor()`: Increment major version (resets minor and patch to 0)
- `void incMinor()`: Increment minor version (resets patch to 0)
- `void incPatch()`: Increment patch version

#### Static Helpers

- `static bool isUpgrade(const char* base, const char* next)`: Check if `next` is an upgrade over `base`

- Arduino String variants available when `ARDUINO` is defined

#### Constants

- `static const size_t MAX_VERSION_LEN`: Maximum allowed length for a version string (default 64, configurable)

## Examples

The library includes 5 comprehensive examples:

1. **BasicUsage** - Start here! Covers parsing, validation, printing, and comparison
2. **PrintableIntegration** - Deep dive into zero-allocation printing
3. **DependencyManagement** - Using `satisfies()` for library compatibility
4. **VersionComparison** - Finding latest/oldest versions with `maximum()`/`minimum()`
5. **FirmwareUpdate** - Complete OTA update decision logic

Find them in: **File** → **Examples** → **SemVerChecker**

## Testing

The project includes comprehensive unit tests covering:

- Strict SemVer 2.0.0 parsing
- Security edge cases (overflow, leading zeros)
- Precedence rules
- Pre-release handling
- All library features

### Run Native Tests (Linux/macOS/WSL)

```bash
# Using g++ (Linux/macOS)
cd tests
g++ -DARDUINO=100 -Itests -Isrc run_tests.cpp -o run_tests && ./run_tests

# Or via WSL on Windows
wsl -e bash -c "g++ -DARDUINO=100 -Itests -Isrc tests/run_tests.cpp -o tests/run_tests && ./tests/run_tests"
```

## Memory Footprint

**Object size:** 82 bytes per `SemVer` instance

- `uint32_t major, minor, patch`: 12 bytes
- `char _buffer[MAX_VERSION_LEN + 1]`: 65 bytes (default 64 char + null terminator)
- Metadata (`_preOffset`, `_buildOffset`, `_valid`): 5 bytes

**Code size:** ~8-9KB Flash (with all features)

- With `-ffunction-sections -Wl,--gc-sections`: only used methods included
- Typical impact on ATmega328P (32KB): ~2-3%
- On ESP32 (4MB): negligible

**Zero heap allocation** - all operations use stack or internal buffer

## Platform Support

Tested and working on:

- ✅ Arduino Uno/Nano (AVR)
- ✅ Arduino Mega
- ✅ ESP8266
- ✅ ESP32
- ✅ STM32
- ✅ Native Linux/macOS (for testing)
- ✅ Any platform with C++11 support

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Author

**bkwoka** - [GitHub](https://github.com/bkwoka)
