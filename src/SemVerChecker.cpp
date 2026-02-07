#include "SemVerChecker.h"

SemVer::SemVer() : major(0), minor(0), patch(0), valid(false) {}

SemVer::SemVer(const String& versionString) : major(0), minor(0), patch(0), valid(false) {
    parse(versionString);
}

SemVer::SemVer(const char* versionString) : major(0), minor(0), patch(0), valid(false) {
    parse(String(versionString));
}

bool SemVer::isValid() const {
    return valid;
}

String SemVer::toString() const {
    if (!valid) return String("");
    
    // Optimization: reserve memory
    String s;
    s.reserve(MAX_VERSION_LEN);
    
    s += major;
    s += '.';
    s += minor;
    s += '.';
    s += patch;

    if (prerelease.length() > 0) {
        s += '-';
        s += prerelease;
    }
    if (build.length() > 0) {
        s += '+';
        s += build;
    }
    return s;
}

void SemVer::parse(const String& inputRaw) {
    // Reset state before parsing
    valid = false;
    major = 0; minor = 0; patch = 0;
    prerelease = ""; build = "";

    if (inputRaw.length() == 0 || inputRaw.length() > MAX_VERSION_LEN) return;

    // Working copy for splitting
    String workStr = inputRaw;
    
    // 1. Extract Build Metadata
    String tmpBuild = "";
    int buildIndex = workStr.indexOf('+');
    if (buildIndex != -1) {
        tmpBuild = workStr.substring(buildIndex + 1);
        if (tmpBuild.length() == 0) return; // Empty build invalid
        workStr = workStr.substring(0, buildIndex);
        
        // Validate build identifiers (leading zeros allowed)
        if (!isValidIdentifier(tmpBuild, true)) return;
    }

    // 2. Extract Prerelease
    String tmpPrerelease = "";
    int preIndex = workStr.indexOf('-');
    if (preIndex != -1) {
        tmpPrerelease = workStr.substring(preIndex + 1);
        if (tmpPrerelease.length() == 0) return; // Empty prerelease invalid
        workStr = workStr.substring(0, preIndex);

        // Validate prerelease identifiers (numeric identifiers MUST NOT include leading zeros)
        if (!isValidIdentifier(tmpPrerelease, false)) return;
    }

    // 3. Parse Core Version (X.Y.Z) - Use local variables for atomicity
    uint32_t tmpMajor, tmpMinor, tmpPatch;

    int dot1 = workStr.indexOf('.');
    if (dot1 == -1) return;
    
    int dot2 = workStr.indexOf('.', dot1 + 1);
    if (dot2 == -1) return;
    
    // Check for excessive dots
    if (workStr.indexOf('.', dot2 + 1) != -1) return;

    if (!parseUint32(workStr.substring(0, dot1), tmpMajor)) return;
    if (!parseUint32(workStr.substring(dot1 + 1, dot2), tmpMinor)) return;
    if (!parseUint32(workStr.substring(dot2 + 1), tmpPatch)) return;

    // 4. Commit changes - only update state if parsing was successful
    this->major = tmpMajor;
    this->minor = tmpMinor;
    this->patch = tmpPatch;
    this->prerelease = tmpPrerelease;
    this->build = tmpBuild;
    this->valid = true;
}

// Validates dot-separated identifiers
bool SemVer::isValidIdentifier(const String& fullId, bool allowLeadingZeros) const {
    if (fullId.length() == 0) return true;
    if (fullId.charAt(fullId.length() - 1) == '.') return false; // Trailing dot

    int start = 0;
    int len = fullId.length();
    
    while (start < len) {
        int end = fullId.indexOf('.', start);
        if (end == -1) end = len;
        
        if (end == start) return false; // Empty segment (.. or starting with .)
        
        String segment = fullId.substring(start, end);
        
        bool isNum = true;
        for (unsigned int i = 0; i < segment.length(); i++) {
            char c = segment.charAt(i);
            if (!isDigit(c) && c != '-') { // SemVer allows '-' in identifiers
                if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))) {
                    return false; // Forbidden character
                }
            }
            if (!isDigit(c)) isNum = false;
        }

        // SemVer: Numeric identifiers MUST NOT include leading zeroes
        if (isNum && !allowLeadingZeros && segment.length() > 1 && segment.charAt(0) == '0') {
            return false;
        }
        
        start = end + 1;
    }
    return true;
}

bool SemVer::isNumeric(const String& s) const {
    if (s.length() == 0) return false;
    for (unsigned int i = 0; i < s.length(); i++) {
        if (!isDigit(s.charAt(i))) return false;
    }
    return true;
}

bool SemVer::parseUint32(const String& s, uint32_t& out) const {
    if (s.length() == 0) return false;
    
    // Check for leading zeros in Core Version (Major/Minor/Patch)
    if (s.length() > 1 && s.charAt(0) == '0') return false;

    if (!isNumeric(s)) return false;
    
    // Safe parsing with overflow detection
    // Use strtoul instead of manual loop for optimization
    char* endPtr;
    unsigned long val = strtoul(s.c_str(), &endPtr, 10);
    
    if (*endPtr != 0) return false; // String is not a number
    if (val > 0xFFFFFFFFUL) return false; // Overflow
    
    out = (uint32_t)val;
    return true;
}

// Comparison implementation
bool SemVer::operator==(const SemVer& other) const {
    // Build metadata is ignored during comparison per SemVer spec
    return valid && other.valid &&
           (major == other.major) && 
           (minor == other.minor) && 
           (patch == other.patch) && 
           (prerelease == other.prerelease);
}

bool SemVer::operator!=(const SemVer& other) const {
    return !(*this == other);
}

bool SemVer::operator<(const SemVer& other) const {
    if (!valid || !other.valid) return false; // Invalid versions are not comparable

    if (major != other.major) return major < other.major;
    if (minor != other.minor) return minor < other.minor;
    if (patch != other.patch) return patch < other.patch;

    // Prerelease logic
    if (prerelease.length() == 0 && other.prerelease.length() > 0) return false; // Stable > Pre
    if (prerelease.length() > 0 && other.prerelease.length() == 0) return true;  // Pre < Stable
    if (prerelease.length() == 0 && other.prerelease.length() == 0) return false; // Equal

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

bool SemVer::isUpgrade(const String& baseVersion, const String& newVersion) {
    SemVer base(baseVersion);
    SemVer newer(newVersion);
    return base.isValid() && newer.isValid() && (base < newer);
}

SemVer SemVer::coerce(const String& rawInput) {
    String versionString = rawInput;
    if (versionString.length() > MAX_VERSION_LEN) return SemVer();

    // Remove "v" prefix
    if (versionString.startsWith("v") || versionString.startsWith("V")) {
        versionString = versionString.substring(1);
    }
    
    // Simple completion of missing parts
    int firstDot = versionString.indexOf('.');
    if (firstDot == -1) {
        // No dot -> append .0.0
        // Respect existing separators
        int sep = -1;
        int dash = versionString.indexOf('-');
        int plus = versionString.indexOf('+');
        if (dash != -1) sep = dash;
        if (plus != -1 && (sep == -1 || plus < sep)) sep = plus;

        if (sep != -1) {
            versionString = versionString.substring(0, sep) + ".0.0" + versionString.substring(sep);
        } else {
            versionString += ".0.0";
        }
    } else {
        // Check if second dot exists before separators
        int secondDot = versionString.indexOf('.', firstDot + 1);
        int sep = -1;
        int dash = versionString.indexOf('-');
        int plus = versionString.indexOf('+');
        if (dash != -1) sep = dash;
        if (plus != -1 && (sep == -1 || plus < sep)) sep = plus;

        if (secondDot == -1 || (sep != -1 && secondDot > sep)) {
             if (sep != -1) {
                versionString = versionString.substring(0, sep) + ".0" + versionString.substring(sep);
            } else {
                versionString += ".0";
            }
        }
    }

    return SemVer(versionString);
}

SemVer::DiffType SemVer::diff(const SemVer& other) const {
    if (!valid || !other.valid) return NONE;
    if (major != other.major) return MAJOR;
    if (minor != other.minor) return MINOR;
    if (patch != other.patch) return PATCH;
    if (prerelease != other.prerelease) return PRERELEASE;
    return NONE;
}

void SemVer::incMajor() {
    if (!valid) return;
    major++;
    minor = 0;
    patch = 0;
    prerelease = "";
    build = "";
}

void SemVer::incMinor() {
    if (!valid) return;
    minor++;
    patch = 0;
    prerelease = "";
    build = "";
}

void SemVer::incPatch() {
    if (!valid) return;
    patch++;
    prerelease = "";
    build = "";
}

int SemVer::comparePrerelease(const String& a, const String& b) const {
    if (a == b) return 0;
    
    int startA = 0, startB = 0;
    int lenA = a.length();
    int lenB = b.length();
    
    while (startA < lenA || startB < lenB) {
        int endA = a.indexOf('.', startA);
        int endB = b.indexOf('.', startB);
        if (endA == -1) endA = lenA;
        if (endB == -1) endB = lenB;

        String partA = (startA < lenA) ? a.substring(startA, endA) : "";
        String partB = (startB < lenB) ? b.substring(startB, endB) : "";

        // If one side is exhausted, the one with more segments has higher precedence
        if (partA.length() == 0 && partB.length() > 0) return -1; // Shorter set < Longer set
        if (partA.length() > 0 && partB.length() == 0) return 1;

        bool aNum = isNumeric(partA);
        bool bNum = isNumeric(partB);

        if (aNum && bNum) {
            // Numeric comparison
            if (partA.length() != partB.length()) {
                return (partA.length() < partB.length()) ? -1 : 1;
            }
            int cmp = partA.compareTo(partB);
            if (cmp != 0) return cmp;
        } else if (aNum && !bNum) {
            return -1; // Numeric < Non-numeric
        } else if (!aNum && bNum) {
            return 1; // Non-numeric > Numeric
        } else {
            // ASCII sort
            int cmp = partA.compareTo(partB);
            if (cmp != 0) return cmp;
        }

        startA = endA + 1;
        startB = endB + 1;
    }
    return 0;
}
