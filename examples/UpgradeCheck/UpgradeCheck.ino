/*
  UpgradeCheck.ino - Example for SemVerChecker library.
  Demonstrates how to use the static isUpgrade method to compare version strings.
*/

#include <SemVerChecker.h>

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println("SemVerChecker Library - Upgrade Check Example");
  Serial.println("=============================================");

  // Helper function to print test results
  auto checkUpgrade = [](String base, String newer, bool expected) {
    bool result = SemVer::isUpgrade(base, newer);
    Serial.print("Checking isUpgrade(\"");
    Serial.print(base);
    Serial.print("\", \"");
    Serial.print(newer);
    Serial.print("\") -> ");
    if (result) Serial.print("TRUE"); else Serial.print("FALSE");
    
    if (result == expected) {
      Serial.println(" [PASS]");
    } else {
      Serial.print(" [FAIL] Expected: ");
      if (expected) Serial.println("TRUE"); else Serial.println("FALSE");
    }
  };

  // Basic comparisons
  checkUpgrade("1.0.0", "1.0.1", true);
  checkUpgrade("1.0.1", "1.0.0", false);
  checkUpgrade("1.0.0", "1.0.0", false); // Equal versions are not an upgrade
  
  // Minor and Major upgrades
  checkUpgrade("1.0.0", "1.1.0", true);
  checkUpgrade("1.1.0", "2.0.0", true);
  checkUpgrade("2.0.0", "1.9.9", false);

  // Pre-release versions
  // 1.0.0-alpha < 1.0.0
  checkUpgrade("1.0.0-alpha", "1.0.0", true);
  // 1.0.0-alpha < 1.0.0-beta
  checkUpgrade("1.0.0-alpha", "1.0.0-beta", true);
  // 1.0.0-beta < 1.0.0-rc.1
  checkUpgrade("1.0.0-beta", "1.0.0-rc.1", true);
  // 1.0.0-rc.1 < 1.0.0
  checkUpgrade("1.0.0-rc.1", "1.0.0", true);

  // Invalid inputs
  checkUpgrade("invalid", "1.0.0", false);
  checkUpgrade("1.0.0", "invalid", false);
  checkUpgrade("invalid", "invalid", false);

  Serial.println("=============================================");
  Serial.println("Tests Completed.");
}

void loop() {
  // Nothing to do here
}
