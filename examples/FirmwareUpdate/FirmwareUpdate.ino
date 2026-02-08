/*
 * FirmwareUpdate.ino - OTA firmware update decision logic
 * 
 * Demonstrates: coerce(), pre-release policy, upgrade check, diff type detection
 */

#include <SemVerChecker.h>

const char* CURRENT_VERSION = "1.5.0";
const char* cloudResponse = "v1.6.0";  // Simulated API response

void setup() {
  Serial.begin(115200);
  while(!Serial);

  SemVer current(CURRENT_VERSION);
  SemVer cloudVer = SemVer::coerce(cloudResponse);

  Serial.print(F("Current: "));
  Serial.println(CURRENT_VERSION);
  Serial.print(F("Available: "));
  Serial.println(cloudVer.toString());

  // Validate server response
  if (!cloudVer.isValid()) {
    Serial.println(F("ERROR: Invalid version from server"));
    return;
  }

  // Skip pre-release versions in production
if (cloudVer.getPrerelease()[0] != '\0') {
    Serial.println(F("SKIP: Pre-release version"));
    return;
}

  // Check if upgrade is available
  if (cloudVer <= current) {
    Serial.println(F("OK: Already up to date"));
    return;
  }

  // Determine update type
  SemVer::DiffType diff = current.diff(cloudVer);
  
  Serial.print(F("UPDATE: "));
  switch (diff) {
    case SemVer::MAJOR:      Serial.println(F("MAJOR - breaking changes")); break;
    case SemVer::MINOR:      Serial.println(F("MINOR - new features")); break;
    case SemVer::PATCH:      Serial.println(F("PATCH - bug fixes")); break;
    case SemVer::PRERELEASE: Serial.println(F("PRERELEASE to stable")); break;
    default:                 Serial.println(F("Unknown")); break;
  }
  
  // Trigger OTA download here...
}

void loop() {}
