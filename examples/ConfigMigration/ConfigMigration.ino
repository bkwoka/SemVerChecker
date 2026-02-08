/*
 * ConfigMigration.ino - Data schema version validation
 * 
 * Demonstrates: MAJOR version check for backward compatibility
 */

#include <SemVerChecker.h>

// Expected config schema major version
const uint32_t REQUIRED_CONFIG_MAJOR = 2;

// Simulated stored config (from EEPROM/Flash)
struct DeviceSettings {
  // Warning: Ensure this buffer is large enough for your versioning scheme.
  // SemVerChecker supports up to 64 chars.
  char schemaVersion[16]; 
} storedData = { "1.9.0" };

void setup() {
  Serial.begin(115200);
  while(!Serial);

  SemVer configVer(storedData.schemaVersion);

  if (!configVer.isValid()) {
    Serial.println(F("ERROR: Invalid config version, reset to defaults"));
    // resetToFactory();
    return;
  }

  Serial.print(F("Config: v"));
  Serial.print(configVer.major);
  Serial.print(F(", Expected: v"));
  Serial.println(REQUIRED_CONFIG_MAJOR);

  // SemVer rule: MAJOR change = breaking change
  if (configVer.major != REQUIRED_CONFIG_MAJOR) {
    Serial.println(F("MIGRATE: Incompatible schema"));
    // runDataMigration(configVer);
  } else {
    Serial.println(F("OK: Schema compatible"));
    // loadConfig();
  }
}

void loop() {}
