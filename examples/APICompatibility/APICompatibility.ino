/*
 * APICompatibility.ino - Version-based feature negotiation
 * 
 * Demonstrates: comparison operators for enabling/disabling features
 */

#include <SemVerChecker.h>

// Minimum version required for advanced features (e.g., encryption)
const char* MIN_VERSION_FOR_ENCRYPTION = "2.1.0";

void setup() {
  Serial.begin(115200);
  while(!Serial);

  // Simulated version received from remote module
  const char* remoteModuleVer = "2.0.9";

  SemVer remote(remoteModuleVer);
  SemVer minRequired(MIN_VERSION_FOR_ENCRYPTION);

  if (!remote.isValid()) {
    Serial.println(F("ERROR: Invalid remote version"));
    return;
  }

  Serial.print(F("Remote: "));
  Serial.print(remote.toString());
  Serial.print(F(", Required: "));
  Serial.println(minRequired.toString());

  // Feature negotiation using comparison
  if (remote >= minRequired) {
    Serial.println(F("MODE: Encrypted (AES-256)"));
    // enableEncryption();
  } else {
    Serial.println(F("MODE: Plaintext (legacy)"));
    // useLegacyProtocol();
  }
}

void loop() {}
