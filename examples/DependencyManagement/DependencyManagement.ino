/*
  DependencyManagement.ino - Version-based dependency and compatibility checking
  
  Demonstrates:
  - Using satisfies() for SemVer caret range logic (NEW!)
  - Library compatibility validation
  - Special 0.x.x handling (minor = breaking change)
  - Module version negotiation
  - Breaking change detection
  
  Real-world scenario:
  IoT device validates dependencies before starting main application.
  Prevents runtime errors from incompatible library versions.
  
  Hardware: Any Arduino board
*/

#include <SemVerChecker.h>

// Application requirements
struct Dependencies {
  const char* name;
  const char* requiredVersion;
  const char* installedVersion;
};

Dependencies deps[] = {
  {"WiFiManager",    "2.0.0",  "2.1.3"},   // Compatible (minor update)
  {"ArduinoJSON",    "6.18.0", "6.18.5"},  // Compatible (patch update)
  {"MQTT",           "1.5.0",  "2.0.0"},   // INCOMPATIBLE (major change)
  {"Sensors",        "0.5.0",  "0.6.0"},   // INCOMPATIBLE (0.x.x minor breaking)
  {"Display",        "0.3.0",  "0.3.2"},   // Compatible (0.x.x patch OK)
};

const int depCount = sizeof(deps) / sizeof(Dependencies);

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println(F("\n=== Dependency Management Demo ===\n"));

  // ============================================
  // 1. Validate All Dependencies
  // ============================================
  Serial.println(F("[1] DEPENDENCY VALIDATION"));
  Serial.println(F("--------------------------------------------------\n"));

  bool allCompatible = true;

  for (int i = 0; i < depCount; i++) {
    Serial.print(F("  "));
    Serial.print(deps[i].name);
    Serial.println(F(":"));

    SemVer required(deps[i].requiredVersion);
    SemVer installed(deps[i].installedVersion);

    // Validate versions
    if (!required.isValid() || !installed.isValid()) {
      Serial.println(F("    ✗ ERROR: Invalid version format"));
      allCompatible = false;
      continue;
    }

    // Show versions
    Serial.print(F("    Required:  >= "));
    Serial.println(required);
    Serial.print(F("    Installed:    "));
    Serial.println(installed);

    // Check compatibility using satisfies()
    // This implements caret range logic: ^X.Y.Z
    if (installed.satisfies(required)) {
      Serial.println(F("    ✓ COMPATIBLE\n"));
    } else {
      Serial.println(F("    ✗ INCOMPATIBLE"));
      
      // Explain why
      if (installed < required) {
        Serial.println(F("       → Version too old"));
      } else if (installed.major != required.major) {
        Serial.println(F("       → Major version mismatch (breaking changes)"));
      } else if (required.major == 0 && installed.minor != required.minor) {
        Serial.println(F("       → 0.x.x: Minor version change is breaking"));
      } else if (required.major == 0 && required.minor == 0 && installed.patch != required.patch) {
        Serial.println(F("       → 0.0.x: Patch version change is breaking"));
      }
      Serial.println();
      
      allCompatible = false;
    }
  }

  // ============================================
  // 2. Explain SemVer Compatibility Rules
  // ============================================
  Serial.println(F("\n[2] SEMVER COMPATIBILITY RULES"));
  Serial.println(F("--------------------------------------------------\n"));
  
  Serial.println(F("  satisfies() implements caret range logic (^):\n"));
  
  Serial.println(F("  For versions >= 1.0.0:"));
  Serial.println(F("    • MAJOR must match (breaking changes)"));
  Serial.println(F("    • MINOR can be higher (backward compatible)"));
  Serial.println(F("    • PATCH can be higher (bug fixes)"));
  Serial.println(F("    Example: 1.5.3 satisfies ^1.2.0 ✓\n"));
  
  Serial.println(F("  For versions 0.x.x (pre-1.0):"));
  Serial.println(F("    • MAJOR must be 0"));
  Serial.println(F("    • MINOR must match (breaking in 0.x.x!)"));
  Serial.println(F("    • PATCH can be higher (EXCEPT for 0.0.x)"));
  Serial.println(F("    Example: 0.3.5 does NOT satisfy ^0.2.0 ✗"));
  Serial.println(F("    Example: 0.0.2 does NOT satisfy ^0.0.1 ✗ (Strict 0.0.x)"));

  // ============================================
  // 3. Demonstrate 0.x.x Special Handling
  // ============================================
  Serial.println(F("\n[3] SPECIAL 0.x.x HANDLING"));
  Serial.println(F("--------------------------------------------------\n"));
  
  struct TestCase {
    const char* installed;
    const char* required;
    bool expected;
  };
  
  TestCase tests[] = {
    {"0.2.5", "0.2.0", true},   // Patch OK
    {"0.3.0", "0.2.0", false},  // Minor breaking!
    {"0.2.0", "0.2.0", true},   // Exact match
    {"1.0.0", "0.9.0", false},  // Major change
  };

  for (const auto& test : tests) {
    SemVer inst(test.installed);
    SemVer req(test.required);
    bool result = inst.satisfies(req);
    
    Serial.print(F("  "));
    Serial.print(inst);
    Serial.print(F(" satisfies ^"));
    Serial.print(req);
    Serial.print(F(" ? "));
    Serial.print(result ? F("YES ✓") : F("NO ✗"));
    
    if (result == test.expected) {
      Serial.println(F(" (correct)"));
    } else {
      Serial.println(F(" (ERROR!)"));
    }
  }

  // ============================================
  // 4. Real-World Application Decision
  // ============================================
  Serial.println(F("\n[4] APPLICATION STARTUP DECISION"));
  Serial.println(F("--------------------------------------------------\n"));

  if (allCompatible) {
    Serial.println(F("  ✓ All dependencies satisfied"));
    Serial.println(F("  → Starting main application..."));
    // startApplication();
  } else {
    Serial.println(F("  ✗ Dependency conflicts detected!"));
    Serial.println(F("  → Application cannot start safely"));
    Serial.println(F("  → Please update incompatible libraries"));
    Serial.println(F("\n  Suggested actions:"));
    Serial.println(F("    1. Update libraries via Library Manager"));
    Serial.println(F("    2. Or downgrade application to compatible version"));
    Serial.println(F("    3. Check library documentation for breaking changes"));
    // haltWithError();
  }

  // ============================================
  // 5. API Version Negotiation Example
  // ============================================
  Serial.println(F("\n[5] API VERSION NEGOTIATION"));
  Serial.println(F("--------------------------------------------------\n"));
  
  Serial.println(F("  Scenario: Connecting to remote module\n"));
  
  SemVer myApiVersion("3.2.0");
  SemVer remoteApiVersion("3.5.1");  // Simulated
  
  Serial.print(F("  Local API:  v"));
  Serial.println(myApiVersion);
  Serial.print(F("  Remote API: v"));
  Serial.println(remoteApiVersion);
  Serial.println();
  
  if (remoteApiVersion.satisfies(myApiVersion)) {
    Serial.println(F("  ✓ APIs compatible - full features available"));
    // useFullProtocol();
  } else if (myApiVersion.satisfies(remoteApiVersion)) {
    Serial.println(F("  ⚠ Local API newer - using legacy mode"));
    // useLegacyProtocol();
  } else {
    Serial.println(F("  ✗ APIs incompatible - connection refused"));
    // refuseConnection();
  }

  Serial.println(F("\n=== Done ===\n"));
}

void loop() {
  // Nothing to do in loop
}
