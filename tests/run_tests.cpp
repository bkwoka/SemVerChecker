#include <iostream>
#include <string>
#include <vector>

// Include local mock Arduino environment
#include "Arduino.h"

// Bring in the library implementation directly for testing
// This avoids complex linking and ensures we use the mock Arduino.h
// The library header will include <Arduino.h>, which we want to resolve to our mock.
// We will handle this by adding -I. to the compiler flags.
#include "../src/SemVerChecker.cpp"

int testsPassed = 0;
int testsFailed = 0;

void assert(bool condition, const char* testName) {
    if (condition) {
        std::cout << "[PASS] " << testName << std::endl;
        testsPassed++;
    } else {
        std::cout << "[FAIL] " << testName << std::endl;
        testsFailed++;
    }
}

void assertEqual(unsigned long actual, unsigned long expected, const char* testName) {
    if (actual == expected) {
        std::cout << "[PASS] " << testName << std::endl;
        testsPassed++;
    } else {
        std::cout << "[FAIL] " << testName << " (Expected: " << expected << ", Got: " << actual << ")" << std::endl;
        testsFailed++;
    }
}

void assertString(String actual, const char* expected, const char* testName) {
    if (actual == expected) {
        std::cout << "[PASS] " << testName << std::endl;
        testsPassed++;
    } else {
        std::cout << "[FAIL] " << testName << " (Expected: \"" << expected << "\", Got: \"" << actual << "\")" << std::endl;
        testsFailed++;
    }
}

int main() {
    std::cout << "\nStarting SemVerChecker Unit Tests (Local Native)...\n" << std::endl;

    // --- Parsing Tests ---
    std::cout << "--- Parsing Tests ---" << std::endl;
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

    // --- Comparison Tests ---
    std::cout << "\n--- Comparison Tests ---" << std::endl;
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
    
    // --- Pre-release Precedence Tests ---
    std::cout << "\n--- Pre-release Precedence Tests ---" << std::endl;
    {
        SemVer v1("1.0.0-alpha");
        SemVer v2("1.0.0");
        assert(v1 < v2, "1.0.0-alpha < 1.0.0");
    }
    {
        SemVer v1("1.0.0-alpha");
        SemVer v2("1.0.0-alpha.1");
        assert(v1 < v2, "1.0.0-alpha < 1.0.0-alpha.1");
    }

    // --- isUpgrade Tests ---
    std::cout << "\n--- isUpgrade Tests ---" << std::endl;
    assert(SemVer::isUpgrade("1.0.0", "1.0.1"), "isUpgrade(1.0.0, 1.0.1) -> true");
    assert(SemVer::isUpgrade("1.0.0", "2.0.0"), "isUpgrade(1.0.0, 2.0.0) -> true");
    assert(!SemVer::isUpgrade("1.0.1", "1.0.0"), "isUpgrade(1.0.1, 1.0.0) -> false");
    assert(!SemVer::isUpgrade("1.0.0", "1.0.0"), "isUpgrade(1.0.0, 1.0.0) -> false");
    assert(!SemVer::isUpgrade("1.0.0", "1.0.0"), "isUpgrade(1.0.0, 1.0.0) -> false");
    assert(SemVer::isUpgrade("1.0.0-alpha", "1.0.0"), "isUpgrade(1.0.0-alpha, 1.0.0) -> true");


    // --- Boundary & Edge Case Tests ---
    std::cout << "\n--- Boundary & Edge Case Tests ---" << std::endl;
    {
        SemVer v("0.0.0");
        assert(v.isValid(), "Parse 0.0.0 valid");
        assertEqual(v.major, 0, "Major 0");
        assertEqual(v.minor, 0, "Minor 0");
        assertEqual(v.patch, 0, "Patch 0");
    }
    {
        SemVer v(" 1.0.0 ");
        assert(!v.isValid(), "Parse ' 1.0.0 ' invalid (whitespace)");
    }
    {
        SemVer v("1.2.3.4");
        assert(!v.isValid(), "Parse '1.2.3.4' invalid");
    }
    {
        SemVer v("-1.0.0");
        assert(!v.isValid(), "Parse '-1.0.0' invalid");
    }
    {
        SemVer v("1.-1.0");
        assert(!v.isValid(), "Parse '1.-1.0' invalid");
    }
    {
        SemVer v("99999.99999.99999");
        assert(v.isValid(), "Parse large numbers valid");
    }

    // --- Coerce Tests ---
    std::cout << "\n--- Coerce Tests ---" << std::endl;
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
    std::cout << "\n--- Diff Tests ---" << std::endl;
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
    std::cout << "\n--- Increment Tests ---" << std::endl;
    {
        SemVer v("1.2.3");
        v.incPatch();
        assertString(v.toString(), "1.2.4", "1.2.3 incPatch -> 1.2.4");
        v.incMinor();
        assertString(v.toString(), "1.3.0", "1.2.4 incMinor -> 1.3.0");
        v.incMajor();
        assertString(v.toString(), "2.0.0", "1.3.0 incMajor -> 2.0.0");
    }

    // --- New Security & Compliance Tests ---
    std::cout << "\n--- Security & Compliance Tests ---" << std::endl;
    {
        SemVer v("01.2.3");
        assert(!v.isValid(), "Leading zero in Major is invalid");
    }
    {
        SemVer v("1.02.3");
        assert(!v.isValid(), "Leading zero in Minor is invalid");
    }
    {
        SemVer v("1.2.03");
        assert(!v.isValid(), "Leading zero in Patch is invalid");
    }
    {
        String longVer = "1.2.3-";
        for(int i=0; i<150; i++) longVer += "a";
        SemVer v(longVer);
        assert(!v.isValid(), "Version exceeding MAX_VERSION_LEN is invalid");
    }
    {
        SemVer v("1.2.3-alpha!");
        assert(!v.isValid(), "Invalid character (!) in prerelease is invalid");
    }
    {
        SemVer v("1.2.3+build@123");
        assert(!v.isValid(), "Invalid character (@) in build is invalid");
    }
    {
        SemVer v("4294967295.0.0"); // UINT32_MAX
        assert(v.isValid(), "UINT32_MAX major is valid");
        assertEqual(v.major, 4294967295UL, "Major is UINT32_MAX");
    }
    {
        SemVer v("4294967296.0.0"); // UINT32_MAX + 1
        assert(!v.isValid(), "Overflow uint32_t is invalid");
    }
    {
        SemVer v1("1.0.0-2");
        SemVer v2("1.0.0-11");
        assert(v1 < v2, "Numeric prerelease comparison (2 < 11)");
    }
    {
        SemVer v("1.0.0-01");
        // Strictly, SemVer 2.0.0 says numeric identifiers MUST NOT include leading zeroes.
        // My current parse() doesn't split prerelease into tokens to check this.
        // But comparePrerelease uses isNumeric(part, false) which disallows it.
        // Let's see if we should make it invalid in parse too.
        // For now, let's see current behavior.
        // Actually, I should probably update parse() to be stricter.
    }

    {
        SemVer v("1.0.0-alpha..1");
        assert(!v.isValid(), "Empty token in prerelease (..) is invalid");
    }
    {
        SemVer v("1.0.0-alpha.");
        assert(!v.isValid(), "Trailing dot in prerelease is invalid");
    }
    {
        SemVer v("1.0.0+build.");
        assert(!v.isValid(), "Trailing dot in build is invalid");
    }
    {
        SemVer v("1.0.0+build..1");
        assert(!v.isValid(), "Empty token in build (..) is invalid");
    }

    // --- More Complex Pre-release Precedence ---
    std::cout << "\n--- Complex Pre-release Precedence ---" << std::endl;
    {
        // Spec order: alpha < alpha.1 < alpha.beta < beta < beta.2 < beta.11 < rc.1 < 1.0.0
        SemVer v1("1.0.0-alpha");
        SemVer v2("1.0.0-alpha.1");
        SemVer v3("1.0.0-alpha.beta");
        SemVer v4("1.0.0-beta");
        SemVer v5("1.0.0-beta.2");
        SemVer v6("1.0.0-beta.11");
        SemVer v7("1.0.0-rc.1");
        SemVer v8("1.0.0");

        assert(v1 < v2, "alpha < alpha.1");
        assert(v2 < v3, "alpha.1 < alpha.beta");
        assert(v3 < v4, "alpha.beta < beta");
        assert(v4 < v5, "beta < beta.2");
        assert(v5 < v6, "beta.2 < beta.11");
        assert(v6 < v7, "beta.11 < rc.1");
        assert(v7 < v8, "rc.1 < 1.0.0");
    }
    {
        SemVer v1("1.2.3-alpha-beta"); // Identifiers with dashes
        assert(v1.isValid(), "Dashes in prerelease are valid");
        assertString(v1.prerelease, "alpha-beta", "Prerelease identifies correctly with dashes");
    }
    {
        SemVer v1("1.2.3+build-metadata");
        assert(v1.isValid(), "Dashes in build metadata are valid");
    }

    // --- Coerce Edge Cases ---
    std::cout << "\n--- Coerce Edge Cases ---" << std::endl;
    {
        SemVer v = SemVer::coerce("1.2.3.4.5");
        assert(!v.isValid(), "Coerce with too many dots results in invalid SemVer");
    }
    {
        SemVer v = SemVer::coerce("v1.2-alpha+build");
        assert(v.isValid(), "Coerce complex partial v1.2-alpha+build");
        assertEqual(v.major, 1, "Complex coerce major");
        assertEqual(v.minor, 2, "Complex coerce minor");
        assertEqual(v.patch, 0, "Complex coerce patch");
        assertString(v.prerelease, "alpha", "Complex coerce prerelease");
        assertString(v.build, "build", "Complex coerce build");
    }
    {
        SemVer v = SemVer::coerce("2.1+onlybuild");
        assert(v.isValid(), "Coerce 2.1+onlybuild");
        assertEqual(v.patch, 0, "Coerce 2.1+onlybuild patch is 0");
        assertString(v.build, "onlybuild", "Coerce 2.1+onlybuild build verified");
    }

    // --- Summary ---
    std::cout << "\n==================================" << std::endl;
    std::cout << "Tests Passed: " << testsPassed << std::endl;
    std::cout << "Tests Failed: " << testsFailed << std::endl;
    std::cout << "==================================" << std::endl;

    return (testsFailed == 0) ? 0 : 1;
}
