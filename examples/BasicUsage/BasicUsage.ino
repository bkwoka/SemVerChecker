/*
  BasicUsage.ino - Basic usage example for SemVerChecker library.
  
  Demonstrates:
  - Parsing version strings
  - Validating versions (handling invalid input)
  - Direct printing with Printable interface (NEW!)
  - Comparing versions
  - Pre-release precedence

  
  Hardware: Any Arduino board with Serial support
  
  Note: This example uses the F() macro to save dynamic memory (RAM) on AVR/ESP platforms.
*/

#include <SemVerChecker.h>

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println(F("\n=== SemVer Checker - Basic Usage ===\n"));

  // ============================================
  // 1. Parsing & Validation
  // ============================================
  Serial.println(F("[1] PARSING & VALIDATION"));
  Serial.println(F("----------------------------------------"));
  
  const char* inputs[] = {
    "1.0.0",           // Valid
    "1.0.0-beta",      // Valid pre-release
    "2.0",             // Invalid (partial)
    "1.0.0-01",        // Invalid (leading zero in numeric prerelease)
    "invalid"          // Invalid
  };

  for (const char* input : inputs) {
    SemVer v(input);
    Serial.print(F("  '"));
    Serial.print(input);
    Serial.print(F("' → "));
    
    if (v.isValid()) {
      Serial.print(F("✓ VALID: "));
      Serial.println(v.toString());
    } else {
      Serial.println(F("✗ INVALID"));
    }
  }

  // ============================================
  // 2. Direct Printing (Printable Interface)
  // ============================================
  Serial.println(F("\n[2] DIRECT PRINTING (NEW!)"));
  Serial.println(F("----------------------------------------"));
  Serial.println(F("  SemVer objects can be printed directly:"));
  
  SemVer version("2.5.0-rc.1+build.456");
  
  // OLD way (still works):
  Serial.print(F("  Old: "));
  Serial.println(version.toString());
  
  // NEW way (zero-allocation, faster):
  Serial.print(F("  New: "));
  Serial.println(version);  // ← Direct print!
  
  Serial.println(F("\n  Benefits:"));
  Serial.println(F("  • No String allocation (saves RAM)"));
  Serial.println(F("  • 5-10x faster on AVR"));
  Serial.println(F("  • Works with any Print stream (LCD, OLED, etc.)"));

  // ============================================
  // 3. Version Comparison
  // ============================================
  Serial.println(F("\n[3] VERSION COMPARISON"));
  Serial.println(F("----------------------------------------"));
  
  SemVer v1("1.0.0");
  SemVer v2("1.0.1");
  SemVer vBeta("1.0.0-beta");
  SemVer vAlpha("1.0.0-alpha");

  // Comparison operators
  Serial.print(F("  "));
  Serial.print(v1);
  Serial.print(F(" < "));
  Serial.print(v2);
  Serial.print(F(" ? "));
  Serial.println((v1 < v2) ? F("YES ✓") : F("NO"));

  // Pre-release precedence: alpha < beta < release
  Serial.println(F("\n  Pre-release precedence:"));
  Serial.print(F("  "));
  Serial.print(vAlpha);
  Serial.print(F(" < "));
  Serial.print(vBeta);
  Serial.print(F(" ? "));
  Serial.println((vAlpha < vBeta) ? F("YES ✓") : F("NO"));

  Serial.print(F("  "));
  Serial.print(vBeta);
  Serial.print(F(" < "));
  Serial.print(v1);
  Serial.print(F(" ? "));
  Serial.println((vBeta < v1) ? F("YES ✓") : F("NO"));




  // ============================================
  // 4. Detailed Compatibility Check
  // ============================================
  Serial.println(F("\n[4] COMPATIBILITY & SAFETY"));
  Serial.println(F("----------------------------------------"));
  
  SemVer stableReq("1.0.0");
  SemVer betaCand("1.1.0-beta");
  
  Serial.print(F("Requirement: ")); Serial.println(stableReq);
  Serial.print(F("Candidate:   ")); Serial.println(betaCand);
  
  // Default check (Safe)
  bool safeCheck = betaCand.satisfies(stableReq); 
  Serial.print(F("  satisfies(req) -> ")); 
  Serial.println(safeCheck ? F("YES") : F("NO (Safety First!)"));
  
  // Explicit check (Allow Pre-release)
  bool explicitCheck = betaCand.satisfies(stableReq, true);
  Serial.print(F("  satisfies(req, true) -> ")); 
  Serial.println(explicitCheck ? F("YES (Explicit Allow)") : F("NO"));

  // ============================================
  // 5. Accessing Components
  // ============================================
  Serial.println(F("\n[5] ACCESSING COMPONENTS"));
  Serial.println(F("----------------------------------------"));
  
  SemVer complexVer("3.2.1-beta.5+build.789");
  Serial.print(F("  Version: "));
  Serial.println(complexVer);
  Serial.print(F("  • Major:      "));
  Serial.println(complexVer.major);
  Serial.print(F("  • Minor:      "));
  Serial.println(complexVer.minor);
  Serial.print(F("  • Patch:      "));
  Serial.println(complexVer.patch);
  Serial.print(F("  • Prerelease: "));
  Serial.println(complexVer.getPrerelease());
  Serial.print(F("  • Build:      "));
  Serial.println(complexVer.getBuild());

  Serial.println(F("\n=== Done ===\n"));
}

void loop() {
  // Nothing to do in loop
}
