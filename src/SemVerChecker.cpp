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
    String s = String(major) + "." + String(minor) + "." + String(patch);
    if (prerelease.length() > 0) {
        s += "-" + prerelease;
    }
    if (build.length() > 0) {
        s += "+" + build;
    }
    return s;
}

void SemVer::parse(String versionString) {
    valid = false;
    major = 0; minor = 0; patch = 0;
    prerelease = ""; build = "";

    if (versionString.length() == 0) return;

    // Remove build metadata first
    int buildIndex = versionString.indexOf('+');
    if (buildIndex != -1) {
        build = versionString.substring(buildIndex + 1);
        versionString = versionString.substring(0, buildIndex);
    }

    // Remove prerelease info
    int preIndex = versionString.indexOf('-');
    if (preIndex != -1) {
        prerelease = versionString.substring(preIndex + 1);
        versionString = versionString.substring(0, preIndex);
    }

    // Parse Major
    int dot1 = versionString.indexOf('.');
    if (dot1 == -1) return; // Invalid format
    String majorStr = versionString.substring(0, dot1);
    if (!isNumeric(majorStr)) return;
    major = majorStr.toInt();

    // Parse Minor
    int dot2 = versionString.indexOf('.', dot1 + 1);
    if (dot2 == -1) {
         // Attempt to parse "Major.Minor" as valid if patch is missing? 
         // SemVer 2.0.0 requires Major.Minor.Patch. strictly.
         // Let's assume strict parsing for now, or just look for the end.
         // Actually, strict SemVer requires 3 numbers.
         return; 
    }
    String minorStr = versionString.substring(dot1 + 1, dot2);
    if (!isNumeric(minorStr)) return;
    minor = minorStr.toInt();

    // Parse Patch
    String patchStr = versionString.substring(dot2 + 1);
    if (!isNumeric(patchStr)) return;
    patch = patchStr.toInt();

    valid = true;
}

bool SemVer::isNumeric(const String& s) const {
    if (s.length() == 0) return false;
    for (unsigned int i = 0; i < s.length(); i++) {
        if (!isDigit(s.charAt(i))) return false;
    }
    return true;
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
    // Remove "v" prefix if present
    if (versionString.startsWith("v") || versionString.startsWith("V")) {
        versionString = versionString.substring(1);
    }
    
    // Handle partial versions like "1" or "1.2"
    int dotCount = 0;
    for (unsigned int i = 0; i < versionString.length(); i++) {
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
        if (versionString.charAt(i) == '.') dotCount++;
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
    
    // Split by dot
    int startA = 0, startB = 0;
    while (startA < a.length() || startB < b.length()) {
        int endA = a.indexOf('.', startA);
        int endB = b.indexOf('.', startB);
        
        if (endA == -1) endA = a.length();
        if (endB == -1) endB = b.length();

        String partA = (startA < a.length()) ? a.substring(startA, endA) : "";
        String partB = (startB < b.length()) ? b.substring(startB, endB) : "";

        // Specification: A larger set of pre-release fields has a higher precedence 
        // than a smaller set, if all of the preceding identifiers are equal.
        if (partA.length() == 0 && partB.length() > 0) return -1; // A ran out first -> A < B
        if (partA.length() > 0 && partB.length() == 0) return 1;  // B ran out first -> A > B

        bool aIsNum = isNumeric(partA);
        bool bIsNum = isNumeric(partB);

        if (aIsNum && bIsNum) {
            // Identifiers consisting of only digits are compared numerically.
            long valA = partA.toInt();
            long valB = partB.toInt();
            if (valA < valB) return -1;
            if (valA > valB) return 1;
        } else if (!aIsNum && bIsNum) {
            // Numeric identifiers always have lower precedence than non-numeric identifiers.
            return 1; 
        } else if (aIsNum && !bIsNum) {
             return -1;
        } else {
            // Identifiers with letters or hyphens are compared lexically in ASCII sort order.
            int cmp = partA.compareTo(partB);
            if (cmp != 0) return cmp;
        }

        startA = endA + 1;
        startB = endB + 1;
    }
    
    return 0;
}
