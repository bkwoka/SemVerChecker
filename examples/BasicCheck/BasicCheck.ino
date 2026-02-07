#include <SemVerChecker.h>

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("SemVer Checker Example");
  Serial.println("----------------------");

  SemVer v1("1.0.0");
  SemVer v2("1.0.1");
  SemVer v3("1.0.0-beta");
  SemVer v4("1.0.0-alpha");
  SemVer v5("2.0.0");

  Serial.print("v1: "); Serial.println(v1.toString());
  Serial.print("v2: "); Serial.println(v2.toString());
  Serial.print("v3: "); Serial.println(v3.toString());
  Serial.print("v4: "); Serial.println(v4.toString());

  // Comparison checks
  if (v1 < v2) {
    Serial.println("1.0.0 < 1.0.1 (Correct)");
  } else {
    Serial.println("Error: 1.0.0 should be < 1.0.1");
  }

  if (v2 > v1) {
    Serial.println("1.0.1 > 1.0.0 (Correct)");
  }

  // Pre-release checks
  // 1.0.0-alpha < 1.0.0-beta < 1.0.0
  if (v4 < v3) {
      Serial.println("1.0.0-alpha < 1.0.0-beta (Correct)");
  } else {
      Serial.println("Error: alpha should be < beta");
  }

  if (v3 < v1) {
      Serial.println("1.0.0-beta < 1.0.0 (Correct)");
  } else {
      Serial.println("Error: beta should be < release");
  }
  
  if (v5 > v2) {
      Serial.println("2.0.0 > 1.0.1 (Correct)");
  }
}

void loop() {
}
