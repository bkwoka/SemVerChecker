#include "SemVerChecker.h"

SemVer::SemVer() : major(0), minor(0), patch(0), valid(false) {}

SemVer::SemVer(String versionString) {
    parse(versionString);
}

SemVer::SemVer(const char* versionString) {
    parse(String(versionString));
}

bool SemVer::isValid() const {
    return valid;
}

String SemVer::toString() const {
    char buf[48]; // Enough for 3x uint32_t + dots + safety
    snprintf(buf, sizeof(buf), "%lu.%lu.%lu", (unsigned long)major, (unsigned long)minor, (unsigned long)patch);
    String s(buf);
    if (prerelease.length() > 0) {
        s += "-";
        s += prerelease;
    }
    if (build.length() > 0) {
        s += "+";
        s += build;
    }
    return s;
}

void SemVer::parse(String versionString) {
    valid = false;
    major = 0; minor = 0; patch = 0;
    prerelease = ""; build = "";

    if (versionString.length() == 0 || (unsigned int)versionString.length() > MAX_VERSION_LEN) return;

    // Remove build metadata first
    int buildIndex = versionString.indexOf('+');
    if (buildIndex != -1) {
        build = versionString.substring(buildIndex + 1);
        if (build.length() == 0 || build.charAt(0) == '.' || build.endsWith(".") || build.indexOf("..") != -1) return;
        for (unsigned int i = 0; i < (unsigned int)build.length(); i++) {
            if (!isValidChar(build.charAt(i), true)) return;
        }
        versionString = versionString.substring(0, buildIndex);
    }

    // Remove prerelease info
    int preIndex = versionString.indexOf('-');
    if (preIndex != -1) {
        prerelease = versionString.substring(preIndex + 1);
        if (prerelease.length() == 0 || prerelease.charAt(0) == '.' || prerelease.endsWith(".") || prerelease.indexOf("..") != -1) return;
        for (unsigned int i = 0; i < (unsigned int)prerelease.length(); i++) {
            if (!isValidChar(prerelease.charAt(i), true)) return;
        }
        versionString = versionString.substring(0, preIndex);
    }

    // Parse Major
    int dot1 = versionString.indexOf('.');
    if (dot1 == -1) return;
    String majorStr = versionString.substring(0, dot1);
    if (!parseUint32(majorStr, major)) return;

    // Parse Minor
    int dot2 = versionString.indexOf('.', dot1 + 1);
    if (dot2 == -1) return;
    String minorStr = versionString.substring(dot1 + 1, dot2);
    if (!parseUint32(minorStr, minor)) return;

    // Parse Patch
    String patchStr = versionString.substring(dot2 + 1);
    if (!parseUint32(patchStr, patch)) return;

    valid = true;
}

bool SemVer::isNumeric(const String& s, bool allowLeadingZeros) const {
    if (s.length() == 0) return false;
    if (!allowLeadingZeros && s.length() > 1 && s.charAt(0) == '0') return false;
    for (unsigned int i = 0; i < s.length(); i++) {
        if (!isDigit(s.charAt(i))) return false;
    }
    return true;
}

bool SemVer::parseUint32(const String& s, uint32_t& out) const {
    if (!isNumeric(s, false)) return false;
    
    unsigned long long val = 0;
    for (unsigned int i = 0; i < s.length(); i++) {
        val = val * 10 + (s.charAt(i) - '0');
        if (val > 0xFFFFFFFFUL) return false; // Overflow uint32_t
    }
    out = (uint32_t)val;
    return true;
}

bool SemVer::isValidChar(char c, bool allowDot) const {
    if (isDigit(c) || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '-') {
        return true;
    }
    if (allowDot && c == '.') return true;
    return false;
}

// Comparison implementation
bool SemVer::operator==(const SemVer& other) const {
    return (major == other.major) && (minor == other.minor) && 
           (patch == other.patch) && (prerelease == other.prerelease);
}

bool SemVer::operator!=(const SemVer& other) const {
    return !(*this == other);
}

bool SemVer::operator<(const SemVer& other) const {
    if (major != other.major) return major < other.major;
    if (minor != other.minor) return minor < other.minor;
    if (patch != other.patch) return patch < other.patch;

    // Pre-release precedence
    // 1. A pre-release version has lower precedence than a normal version.
    if (prerelease.length() == 0 && other.prerelease.length() > 0) return false; // This is normal, other is pre -> This > Other
    if (prerelease.length() > 0 && other.prerelease.length() == 0) return true;  // This is pre, other is normal -> This < Other
    if (prerelease.length() == 0 && other.prerelease.length() == 0) return false; // Both normal, equal

    // Compare pre-release identifiers
    return comparePrerelease(prerelease, other.prerelease) < 0;
}

bool SemVer::operator>(const SemVer& other) const {
    return other < *this;
}

bool SemVer::operator<=(const SemVer& other) const {
    return !(*this > other);
}

bool SemVer::operator>=(const SemVer& other) const {
    return !(*this < other);
}

bool SemVer::isUpgrade(String baseVersion, String newVersion) {
    SemVer base(baseVersion);
    SemVer newer(newVersion);
    return base.isValid() && newer.isValid() && (base < newer);
}

SemVer SemVer::coerce(String versionString) {
    if (versionString.length() > MAX_VERSION_LEN) return SemVer("");

    // Remove "v" prefix if present
    if (versionString.startsWith("v") || versionString.startsWith("V")) {
        versionString = versionString.substring(1);
    }
    
    // Handle partial versions like "1" or "1.2"
    int dotCount = 0;
    for (unsigned int i = 0; i < (unsigned int)versionString.length(); i++) {
        if (versionString.charAt(i) == '.') dotCount++;
    }

    // Check if it has prerelease/build metadata which might confuse dot counting
    int dashIndex = versionString.indexOf('-');
    int plusIndex = versionString.indexOf('+');
    int stopIndex = versionString.length();
    if (dashIndex != -1 && dashIndex < stopIndex) stopIndex = dashIndex;
    if (plusIndex != -1 && plusIndex < stopIndex) stopIndex = plusIndex;
    
    // Recount dots only in the version core
    dotCount = 0;
    for (int i = 0; i < stopIndex; i++) {
        if (versionString.charAt((size_t)i) == '.') dotCount++;
    }

    if (dotCount == 0) versionString = versionString.substring(0, stopIndex) + ".0.0" + versionString.substring(stopIndex);
    else if (dotCount == 1) versionString = versionString.substring(0, stopIndex) + ".0" + versionString.substring(stopIndex);

    return SemVer(versionString);
}

SemVer::DiffType SemVer::diff(const SemVer& other) const {
    if (major != other.major) return MAJOR;
    if (minor != other.minor) return MINOR;
    if (patch != other.patch) return PATCH;
    if (prerelease != other.prerelease) return PRERELEASE;
    return NONE;
}

void SemVer::incMajor() {
    major++;
    minor = 0;
    patch = 0;
    prerelease = "";
    build = "";
}

void SemVer::incMinor() {
    minor++;
    patch = 0;
    prerelease = "";
    build = "";
}

void SemVer::incPatch() {
    patch++;
    prerelease = "";
    build = "";
}

int SemVer::comparePrerelease(const String& a, const String& b) const {
    if (a == b) return 0;
    
    int startA = 0, startB = 0;
    unsigned int lenA = (unsigned int)a.length();
    unsigned int lenB = (unsigned int)b.length();
    while ((unsigned int)startA < lenA || (unsigned int)startB < lenB) {
        int endA = a.indexOf('.', startA);
        int endB = b.indexOf('.', startB);
        
        if (endA == -1) endA = a.length();
        if (endB == -1) endB = b.length();

        String partA = (startA < a.length()) ? a.substring(startA, endA) : "";
        String partB = (startB < b.length()) ? b.substring(startB, endB) : "";

        if (partA.length() == 0 && partB.length() > 0) return -1;
        if (partA.length() > 0 && partB.length() == 0) return 1;

        // SemVer: Numeric identifiers MUST NOT include leading zeroes.
        // If it's all digits but has leading zero, it's treated as non-numeric per some interpretations,
        // but strictly it's invalid. Our parse() should probably have caught it if it was strict.
        // For comparison, let's see if they are numeric (no leading zeros allowed).
        bool aIsNum = isNumeric(partA, false);
        bool bIsNum = isNumeric(partB, false);

        if (aIsNum && bIsNum) {
            uint32_t valA, valB;
            parseUint32(partA, valA);
            parseUint32(partB, valB);
            if (valA < valB) return -1;
            if (valA > valB) return 1;
        } else if (!aIsNum && bIsNum) {
            return 1; 
        } else if (aIsNum && !bIsNum) {
             return -1;
        } else {
            int cmp = partA.compareTo(partB);
            if (cmp != 0) return cmp;
        }

        startA = ((unsigned int)endA < lenA) ? endA + 1 : (int)lenA;
        startB = ((unsigned int)endB < lenB) ? endB + 1 : (int)lenB;
    }
    
    return 0;
}
