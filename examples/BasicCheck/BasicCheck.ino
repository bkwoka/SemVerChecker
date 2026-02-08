/*
  BasicCheck.ino - Basic usage example for SemVerChecker library.
  Demonstrates:
  - Parsing version strings
  - Validating versions (handling invalid input)
  - Comparing versions
  - Pre-release precedence
  - Using coerce() to handle partial/loose inputs
  
  Note: This example uses the F() macro for strings to save dynamic memory (RAM) on AVR/ESP platforms.
*/

#include <SemVerChecker.h>

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println(F("\n--- SemVer Checker Basic Example ---"));

  // 1. Parsing & Validation
  Serial.println(F("\n[1] Parsing & Validation"));
  
  const char* inputs[] = {
    "1.0.0",           // Valid
    "1.0.0-beta",      // Valid pre-release
    "2.0",             // Invalid (partial)
    "1.0.0-01",        // Invalid (leading zero in numeric prerelease)
    "invalid"          // Invalid
  };

  for (const char* input : inputs) {
    SemVer v(input);
    Serial.print(F("Parsing '"));
    Serial.print(input);
    Serial.print(F("': "));
    if (v.isValid()) {
      Serial.print(F("VALID -> "));
      Serial.println(v.toString());
    } else {
      Serial.println(F("INVALID"));
    }
  }

  // 2. Comparison
  Serial.println(F("\n[2] Comparison"));
  SemVer v1("1.0.0");
  SemVer v2("1.0.1");
  SemVer vBeta("1.0.0-beta");
  SemVer vAlpha("1.0.0-alpha");

  if (v1.isValid() && v2.isValid()) {
    Serial.print(v1.toString());
    Serial.print(F(" < "));
    Serial.print(v2.toString());
    Serial.print(F(" ? "));
    Serial.println((v1 < v2) ? F("YES") : F("NO")); // Expected: YES
  }

  // Pre-release precedence: alpha < beta < release
  Serial.print(vAlpha.toString());
  Serial.print(F(" < "));
  Serial.print(vBeta.toString());
  Serial.print(F(" ? "));
  Serial.println((vAlpha < vBeta) ? F("YES") : F("NO")); // Expected: YES

  Serial.print(vBeta.toString());
  Serial.print(F(" < "));
  Serial.print(v1.toString());
  Serial.print(F(" ? "));
  Serial.println((vBeta < v1) ? F("YES") : F("NO"));   // Expected: YES (pre-release < normal)

  // 3. Coercion (Handling User Input)
  // Useful when you receive version strings that might be "lazy" (e.g. "v1.2" instead of "1.2.0")
  Serial.println(F("\n[3] Coercion (Loose Parsing)"));
  
  const char* looseInputs[] = {
    "v1.2.3",
    "1.2",
    "1",
    "v2.0-rc.1"
  };

  for (const char* input : looseInputs) {
    SemVer v = SemVer::coerce(input);
    Serial.print(F("Coercing '"));
    Serial.print(input);
    Serial.print(F("': "));
    if (v.isValid()) {
      Serial.print(F("VALID -> "));
      Serial.println(v.toString());
    } else {
      Serial.println(F("INVALID"));
    }
  }

  Serial.println(F("\n--- Done ---"));
}

void loop() {
}
