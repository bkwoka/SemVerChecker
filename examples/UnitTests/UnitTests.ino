/*
  UnitTests.ino - Comprehensive unit tests for SemVerChecker library.
*/

#include <SemVerChecker.h>

int testsPassed = 0;
int testsFailed = 0;

void assert(bool condition, const char* testName) {
  if (condition) {
    Serial.print("[PASS] ");
    Serial.println(testName);
    testsPassed++;
  } else {
    Serial.print("[FAIL] ");
    Serial.println(testName);
    testsFailed++;
  }
}

void assertEqual(int actual, int expected, const char* testName) {
  if (actual == expected) {
    Serial.print("[PASS] ");
    Serial.println(testName);
    testsPassed++;
  } else {
    Serial.print("[FAIL] ");
    Serial.print(testName);
    Serial.print(" (Expected: ");
    Serial.print(expected);
    Serial.print(", Got: ");
    Serial.print(actual);
    Serial.println(")");
    testsFailed++;
  }
}

void assertString(String actual, String expected, const char* testName) {
  if (actual == expected) {
    Serial.print("[PASS] ");
    Serial.println(testName);
    testsPassed++;
  } else {
    Serial.print("[FAIL] ");
    Serial.print(testName);
    Serial.print(" (Expected: \"");
    Serial.print(expected);
    Serial.print("\", Got: \"");
    Serial.print(actual);
    Serial.println("\")");
    testsFailed++;
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }
  Serial.println("\nStarting SemVerChecker Unit Tests...\n");

  // --- Parsing Tests ---
  Serial.println("--- Parsing Tests ---");
  {
    SemVer v("1.2.3");
    assert(v.isValid(), "Parse 1.2.3 valid");
    assertEqual(v.major, 1, "Parse 1.2.3 major");
    assertEqual(v.minor, 2, "Parse 1.2.3 minor");
    assertEqual(v.patch, 3, "Parse 1.2.3 patch");
    assertString(v.prerelease, "", "Parse 1.2.3 prerelease empty");
    assertString(v.build, "", "Parse 1.2.3 build empty");
  }
  {
    SemVer v("10.20.30-alpha.1+build.123");
    assert(v.isValid(), "Parse complex version valid");
    assertEqual(v.major, 10, "Complex major");
    assertEqual(v.minor, 20, "Complex minor");
    assertEqual(v.patch, 30, "Complex patch");
    assertString(v.prerelease, "alpha.1", "Complex prerelease");
    assertString(v.build, "build.123", "Complex build");
  }
  {
    SemVer v("invalid");
    assert(!v.isValid(), "Parse 'invalid' returns invalid");
  }
  {
    SemVer v("1.2");
    assert(!v.isValid(), "Parse incomplete '1.2' returns invalid");
  }

  // --- Comparison Tests ---
  Serial.println("\n--- Comparison Tests ---");
  {
    SemVer v1("1.0.0");
    SemVer v2("2.0.0");
    assert(v1 < v2, "1.0.0 < 2.0.0");
    assert(v2 > v1, "2.0.0 > 1.0.0");
    assert(v1 != v2, "1.0.0 != 2.0.0");
  }
  {
    SemVer v1("1.1.0");
    SemVer v2("1.2.0");
    assert(v1 < v2, "1.1.0 < 1.2.0");
  }
  {
    SemVer v1("1.0.1");
    SemVer v2("1.0.2");
    assert(v1 < v2, "1.0.1 < 1.0.2");
  }
  {
    SemVer v1("1.0.0");
    SemVer v2("1.0.0");
    assert(v1 == v2, "1.0.0 == 1.0.0");
    assert(v1 >= v2, "1.0.0 >= 1.0.0");
    assert(v1 <= v2, "1.0.0 <= 1.0.0");
  }

  // --- Pre-release Precedence Tests ---
  Serial.println("\n--- Pre-release Precedence Tests ---");
  {
    // 1.0.0-alpha < 1.0.0
    SemVer v1("1.0.0-alpha");
    SemVer v2("1.0.0");
    assert(v1 < v2, "1.0.0-alpha < 1.0.0");
  }
  {
    // 1.0.0-alpha < 1.0.0-alpha.1
    SemVer v1("1.0.0-alpha");
    SemVer v2("1.0.0-alpha.1");
    assert(v1 < v2, "1.0.0-alpha < 1.0.0-alpha.1");
  }
  {
    // 1.0.0-alpha.1 < 1.0.0-alpha.beta
    SemVer v1("1.0.0-alpha.1");
    SemVer v2("1.0.0-alpha.beta");
    assert(v1 < v2, "1.0.0-alpha.1 < 1.0.0-alpha.beta");
  }
  {
    // 1.0.0-beta < 1.0.0-beta.2
    SemVer v1("1.0.0-beta");
    SemVer v2("1.0.0-beta.2");
    assert(v1 < v2, "1.0.0-beta < 1.0.0-beta.2");
  }
  {
    // 1.0.0-beta.2 < 1.0.0-beta.11 (Numeric comparison)
    SemVer v1("1.0.0-beta.2");
    SemVer v2("1.0.0-beta.11");
    assert(v1 < v2, "1.0.0-beta.2 < 1.0.0-beta.11");
  }
  {
    // 1.0.0-rc.1 < 1.0.0
    SemVer v1("1.0.0-rc.1");
    SemVer v2("1.0.0");
    assert(v1 < v2, "1.0.0-rc.1 < 1.0.0");
  }

  // --- isUpgrade Tests ---
  Serial.println("\n--- isUpgrade Tests ---");
  assert(SemVer::isUpgrade("1.0.0", "1.0.1"), "isUpgrade(1.0.0, 1.0.1) -> true");
  assert(SemVer::isUpgrade("1.0.0", "2.0.0"), "isUpgrade(1.0.0, 2.0.0) -> true");
  assert(!SemVer::isUpgrade("1.0.1", "1.0.0"), "isUpgrade(1.0.1, 1.0.0) -> false");
  assert(!SemVer::isUpgrade("1.0.0", "1.0.0"), "isUpgrade(1.0.0, 1.0.0) -> false");
  assert(SemVer::isUpgrade("1.0.0-alpha", "1.0.0"), "isUpgrade(1.0.0-alpha, 1.0.0) -> true");
  assert(!SemVer::isUpgrade("invalid", "1.0.0"), "isUpgrade(invalid, 1.0.0) -> false");
  assert(!SemVer::isUpgrade("1.0.0", "invalid"), "isUpgrade(1.0.0, invalid) -> false");


  // --- Boundary & Edge Case Tests ---
  Serial.println("\n--- Boundary & Edge Case Tests ---");
  {
      SemVer v("0.0.0");
      assert(v.isValid(), "Parse 0.0.0 valid");
      assertEqual(v.major, 0, "Major 0");
      assertEqual(v.minor, 0, "Minor 0");
      assertEqual(v.patch, 0, "Patch 0");
  }
  {
      SemVer v(" 1.0.0 ");
      // The current implementation might be lenient or strict with whitespace. 
      // SemVer spec doesn't allow whitespace, but `toInt()` might ignore leading.
      // Let's test current behavior. If strict, this should be invalid.
      // Based on code, `isNumeric` loops through checking `isDigit`. Spaces are not digits.
      // So " 1" is not numeric.
      assert(!v.isValid(), "Parse ' 1.0.0 ' invalid (whitespace)");
  }
  {
      SemVer v("1.2.3.4");
      // Should be invalid as it has 4 numbers
      // Code logic: dot1 found, dot2 found. patchStr = "3.4". isNumeric("3.4") -> dot is not digit -> false.
      assert(!v.isValid(), "Parse '1.2.3.4' invalid");
  }
  {
      SemVer v("-1.0.0");
      // " -1" is not numeric (isDigit('-') is false)
      assert(!v.isValid(), "Parse '-1.0.0' invalid");
  }
  {
      SemVer v("1.-1.0");
      assert(!v.isValid(), "Parse '1.-1.0' invalid");
  }
  {
      SemVer v("99999.99999.99999");
      assert(v.isValid(), "Parse large numbers valid");
      // Note: integer overflow might occur if really large, but 99999 fits in int.
  }


  // --- Coerce Tests ---
  Serial.println("\n--- Coerce Tests ---");
  {
      SemVer v = SemVer::coerce("v1.2.3");
      assert(v.isValid(), "Coerce v1.2.3 valid");
      assertEqual(v.major, 1, "Major 1");
  }
  {
      SemVer v = SemVer::coerce("1.2");
      assert(v.isValid(), "Coerce 1.2 valid");
      assertEqual(v.major, 1, "Major 1");
      assertEqual(v.minor, 2, "Minor 2");
      assertEqual(v.patch, 0, "Patch 0");
  }
  {
      SemVer v = SemVer::coerce("1");
      assert(v.isValid(), "Coerce 1 valid");
      assertEqual(v.major, 1, "Major 1");
      assertEqual(v.minor, 0, "Minor 0");
      assertEqual(v.patch, 0, "Patch 0");
  }

  // --- Diff Tests ---
  Serial.println("\n--- Diff Tests ---");
  {
      SemVer v1("1.0.0");
      SemVer v2("2.0.0");
      assert(v1.diff(v2) == SemVer::MAJOR, "Diff 1.0.0 vs 2.0.0 is MAJOR");
  }
  {
      SemVer v1("1.0.0");
      SemVer v2("1.1.0");
      assert(v1.diff(v2) == SemVer::MINOR, "Diff 1.0.0 vs 1.1.0 is MINOR");
  }
  {
      SemVer v1("1.0.0");
      SemVer v2("1.0.1");
      assert(v1.diff(v2) == SemVer::PATCH, "Diff 1.0.0 vs 1.0.1 is PATCH");
  }

  // --- Increment Tests ---
  Serial.println("\n--- Increment Tests ---");
  {
      SemVer v("1.2.3");
      v.incPatch();
      assertString(v.toString(), "1.2.4", "1.2.3 incPatch -> 1.2.4");
      v.incMinor();
      assertString(v.toString(), "1.3.0", "1.2.4 incMinor -> 1.3.0");
      v.incMajor();
      assertString(v.toString(), "2.0.0", "1.3.0 incMajor -> 2.0.0");
  }

  // --- Summary ---
  Serial.println("\n==================================");
  Serial.print("Tests Passed: ");
  Serial.println(testsPassed);
  Serial.print("Tests Failed: ");
  Serial.println(testsFailed);
  Serial.println("==================================");
}

void loop() {
  // Do nothing
}
