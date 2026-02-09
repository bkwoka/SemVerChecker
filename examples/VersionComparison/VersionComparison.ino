/*
  VersionComparison.ino - Finding max/min versions
  
  Demonstrates:
  - SemVer::maximum() - find the greater of two versions (NEW!)
  - SemVer::minimum() - find the lesser of two versions (NEW!)
  - Practical use cases (selecting compatible versions)
  - Handling invalid versions gracefully
  
  Real-world scenarios:
  - Selecting best compatible library version
  - Finding minimum supported version across devices
  - Determining latest stable release
  
  Hardware: Any Arduino board
*/

#include <SemVerChecker.h>

// Simulated version data
const char* firmwareVersions[] = {
  "2.1.0",
  "2.3.5",
  "2.2.1",
  "2.3.0",
  "1.9.7"
};

const char* deviceVersions[] = {
  "1.5.0",  // Device A
  "1.7.2",  // Device B
  "1.6.0",  // Device C
  "2.0.0"   // Device D (newer generation)
};

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println(F("\n=== Version Comparison Demo ===\n"));

  // ============================================
  // 1. Basic maximum() and minimum()
  // ============================================
  Serial.println(F("[1] BASIC COMPARISON"));
  Serial.println(F("----------------------------------------\n"));

  SemVer v1("1.2.3");
  SemVer v2("1.3.0");

  SemVer latest = SemVer::maximum(v1, v2);
  SemVer oldest = SemVer::minimum(v1, v2);

  Serial.print(F("  Comparing: "));
  Serial.print(v1);
  Serial.print(F(" vs "));
  Serial.println(v2);
  Serial.println();
  
  Serial.print(F("  Maximum: "));
  Serial.println(latest);
  
  Serial.print(F("  Minimum: "));
  Serial.println(oldest);

  // ============================================
  // 2. Finding Latest Firmware
  // ============================================
  Serial.println(F("\n[2] FINDING LATEST FIRMWARE"));
  Serial.println(F("----------------------------------------\n"));

  Serial.println(F("  Available firmware versions:"));
  
  SemVer newestFirmware("0.0.0");  // Start with minimal version
  
  for (int i = 0; i < 5; i++) {
    SemVer current(firmwareVersions[i]);
    
    Serial.print(F("    - "));
    Serial.println(current);
    
    if (current.isValid()) {
      newestFirmware = SemVer::maximum(newestFirmware, current);
    }
  }
  
  Serial.println();
  Serial.print(F("  → Latest version: "));
  Serial.println(newestFirmware);

  // ============================================
  // 3. Finding Minimum Compatible Version
  // ============================================
  Serial.println(F("\n[3] MINIMUM COMPATIBLE VERSION"));
  Serial.println(F("----------------------------------------\n"));

  Serial.println(F("  Device versions in fleet:"));
  
  SemVer minDeviceVersion("999.999.999");  // Start with max possible
  
  for (int i = 0; i < 4; i++) {
    SemVer current(deviceVersions[i]);
    
    Serial.print(F("    Device "));
    Serial.print((char)('A' + i));
    Serial.print(F(": "));
    Serial.println(current);
    
    if (current.isValid()) {
      minDeviceVersion = SemVer::minimum(minDeviceVersion, current);
    }
  }
  
  Serial.println();
  Serial.print(F("  → Oldest device: "));
  Serial.println(minDeviceVersion);
  Serial.println(F("  → Server must support this version or newer"));

  // ============================================
  // 4. Selecting Best Compatible Version
  // ============================================
  Serial.println(F("\n[4] SELECTING COMPATIBLE VERSION"));
  Serial.println(F("----------------------------------------\n"));

  // Scenario: Find newest firmware compatible with oldest device
  SemVer targetVersion("1.5.0");  // Minimum requirement
  SemVer compatible("0.0.0");
  
  Serial.print(F("  Target minimum: "));
  Serial.println(targetVersion);
  Serial.println(F("\n  Scanning firmware versions:"));
  
  for (int i = 0; i < 5; i++) {
    SemVer fw(firmwareVersions[i]);
    
    if (!fw.isValid()) continue;
    
    // Check if firmware satisfies target and is better than current choice
    if (fw.satisfies(targetVersion)) {
      Serial.print(F("    "));
      Serial.print(fw);
      Serial.println(F(" ✓ compatible"));
      compatible = SemVer::maximum(compatible, fw);
    } else {
      Serial.print(F("    "));
      Serial.print(fw);
      Serial.println(F(" ✗ incompatible"));
    }
  }
  
  Serial.println();
  if (compatible.major > 0 || compatible.minor > 0 || compatible.patch > 0) {
    Serial.print(F("  → Best choice: "));
    Serial.println(compatible);
  } else {
    Serial.println(F("  → No compatible version found!"));
  }

  // ============================================
  // 5. Handling Invalid Versions
  // ============================================
  Serial.println(F("\n[5] INVALID VERSION HANDLING"));
  Serial.println(F("----------------------------------------\n"));

  SemVer valid("2.0.0");
  SemVer invalid("not-a-version");

  Serial.print(F("  Valid version:   "));
  Serial.println(valid);
  Serial.print(F("  Invalid version: "));
  Serial.println(invalid);
  Serial.println();

  // maximum() and minimum() handle invalid versions gracefully
  SemVer result1 = SemVer::maximum(valid, invalid);
  SemVer result2 = SemVer::minimum(valid, invalid);

  Serial.print(F("  maximum(valid, invalid) = "));
  Serial.print(result1);
  Serial.println(F(" (returns valid)"));

  Serial.print(F("  minimum(valid, invalid) = "));
  Serial.print(result2);
  Serial.println(F(" (returns valid)"));

  // Both invalid
  SemVer invalid2("also-invalid");
  SemVer result3 = SemVer::maximum(invalid, invalid2);
  
  Serial.print(F("\n  maximum(invalid, invalid) = "));
  if (result3.isValid()) {
    Serial.println(result3);
  } else {
    Serial.println(F("invalid (both were invalid)"));
  }

  // ============================================
  // 6. Practical Helper Function
  // ============================================
  Serial.println(F("\n[6] HELPER FUNCTION EXAMPLE"));
  Serial.println(F("----------------------------------------\n"));

  Serial.println(F("  Example: findLatestStable()\n"));
  Serial.println(F("  // Find newest non-prerelease version"));
  Serial.println(F("  SemVer findLatestStable(SemVer versions[], int count) {"));
  Serial.println(F("    SemVer latest(\"0.0.0\");"));
  Serial.println(F("    for (int i = 0; i < count; i++) {"));
  Serial.println(F("      if (versions[i].getPrerelease()[0] == '\\0') {"));
  Serial.println(F("        latest = SemVer::maximum(latest, versions[i]);"));
  Serial.println(F("      }"));
  Serial.println(F("    }"));
  Serial.println(F("    return latest;"));
  Serial.println(F("  }"));

  // ============================================
  // 7. Pre-release Comparison
  // ============================================
  Serial.println(F("\n[7] PRE-RELEASE VERSIONS"));
  Serial.println(F("----------------------------------------\n"));

  SemVer stable("2.0.0");
  SemVer rc("2.0.0-rc.1");
  SemVer beta("2.0.0-beta");

  Serial.println(F("  Comparing releases:"));
  Serial.print(F("    Stable: "));
  Serial.println(stable);
  Serial.print(F("    RC:     "));
  Serial.println(rc);
  Serial.print(F("    Beta:   "));
  Serial.println(beta);
  Serial.println();

  SemVer newest = SemVer::maximum(SemVer::maximum(stable, rc), beta);
  
  Serial.print(F("  → Newest: "));
  Serial.print(newest);
  Serial.println(F(" (stable > rc > beta)"));

  Serial.println(F("\n=== Done ===\n"));
}

void loop() {
  // Nothing to do in loop
}
