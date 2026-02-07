#include "SemVerChecker.h"

SemVer::SemVer() : major(0), minor(0), patch(0), prerelease(""), build(""), valid(false) {}

SemVer::SemVer(const String& versionString) : major(0), minor(0), patch(0), prerelease(""), build(""), valid(false) {
    parse(versionString);
}

SemVer::SemVer(const char* versionString) : major(0), minor(0), patch(0), prerelease(""), build(""), valid(false) {
    parse(String(versionString));
}

void SemVer::parse(const String& inputRaw) {
    // 1. Basic Guards
    if (!basicGuards(inputRaw)) {
        valid = false;
        return;
    }

    // 2. Split Main Parts
    int dot1, dot2, hyphenIndex, plusIndex;
    if (!splitMainParts(inputRaw, dot1, dot2, hyphenIndex, plusIndex)) {
        valid = false;
        return;
    }

    // Determine boundaries based on separators
    int endOfPatch = (hyphenIndex != -1) ? hyphenIndex : ((plusIndex != -1) ? plusIndex : inputRaw.length());
    int startOfPre = (hyphenIndex != -1) ? hyphenIndex + 1 : -1;
    int endOfPre   = (plusIndex != -1) ? plusIndex : inputRaw.length();
    int startOfBuild = (plusIndex != -1) ? plusIndex + 1 : -1;
    int endOfBuild = inputRaw.length();

    // 3. Validate Core Structure (Regex: 0|[1-9]\d*)
    if (!validateCore(inputRaw, dot1, dot2, endOfPatch)) {
        valid = false;
        return;
    }

    // 4. Validate Prerelease (Regex rules for identifiers)
    if (startOfPre != -1) {
        if (!validatePrerelease(inputRaw, startOfPre, endOfPre)) {
            valid = false;
            return;
        }
    }

    // 5. Validate Build Metadata (Regex rules)
    if (startOfBuild != -1) {
        if (!validateBuild(inputRaw, startOfBuild, endOfBuild)) {
            valid = false;
            return;
        }
    }

    // 6. Parse Core Values (Detect Overflow)
    uint32_t tmpMajor, tmpMinor, tmpPatch;
    if (!parseCore(inputRaw, dot1, dot2, endOfPatch, tmpMajor, tmpMinor, tmpPatch)) {
        valid = false;
        return;
    }

    // 7. Commit
    // Extract substrings only now that validity is guaranteed
    String tmpPreStr = (startOfPre != -1) ? inputRaw.substring(startOfPre, endOfPre) : "";
    String tmpBuildStr = (startOfBuild != -1) ? inputRaw.substring(startOfBuild, endOfBuild) : "";
    
    commit(tmpMajor, tmpMinor, tmpPatch, tmpPreStr, tmpBuildStr);
}

bool SemVer::basicGuards(const String& input) const {
    if (input.length() == 0 || input.length() > MAX_VERSION_LEN) return false;
    return true;
}

bool SemVer::splitMainParts(const String& input, int& dot1, int& dot2, int& hyphen, int& plus) const {
    dot1 = input.indexOf('.');
    if (dot1 == -1) return false;

    dot2 = input.indexOf('.', dot1 + 1);
    if (dot2 == -1) return false;

    // According to regex: ^... core ... (?:-(...))? (?:\+(...))? $
    // The hyphen for prerelease must come after the patch number.
    // However, hyphens can exist IN prerelease or build.
    // Logic: Look for first + (build). Prerelease is between patch and build.
    
    plus = input.indexOf('+');
    
    // Search for hyphen specifically after the patch dot.
    // It must be the first separator after dot2.
    int scanStart = dot2 + 1;
    hyphen = -1;
    
    for (unsigned int i = scanStart; i < input.length(); i++) {
        char c = input.charAt(i);
        if (c == '+') {
            // Found build start before any hyphen -> no prerelease
            break; 
        }
        if (c == '-') {
            hyphen = i;
            break;
        }
        // If we hit a non-digit before a separator, it might be invalid core, 
        // but we handle validation later. However, structure dictates:
        // Major.Minor.Patch[-Pre][+Build]
        // Patch must be digits. So if we see non-digit that isn't + or -, it's invalid unless it's part of Pre/Build later.
        // Actually, strictly finding the delimiters:
        if (!isDigit(c)) {
             // If we find a non-digit that is NOT + or -, and we haven't found hyphen yet,
             // then this version string is structurally invalid for the "Patch" segment
             // UNLESS it marks the start of something. But standard says Patch is Digits only.
             // So any non-digit terminates Patch.
             // If it's not - or +, it's an error in strict mode.
             return false;
        }
    }
    
    // Sanity check indices
    if (hyphen != -1 && plus != -1 && hyphen > plus) return false; // - inside build is ok, but we looked from start
    
    // Empty segments check (e.g. "1..2")
    if (dot1 == 0 || (dot2 - dot1) <= 1) return false;
    
    return true;
}

bool SemVer::validateCore(const String& input, int dot1, int dot2, int endOfPatch) const {
    // Check Major
    if (!checkSegment(input, 0, dot1, false)) return false;
    // Check Minor
    if (!checkSegment(input, dot1 + 1, dot2, false)) return false;
    // Check Patch
    if (!checkSegment(input, dot2 + 1, endOfPatch, false)) return false;
    return true;
}

bool SemVer::validatePrerelease(const String& input, int start, int end) const {
    if (start >= end) return false; // Empty prerelease specified (e.g. "1.2.3-")

    int current = start;
    while (current < end) {
        int nextDot = input.indexOf('.', current);
        if (nextDot == -1 || nextDot > end) nextDot = end;
        
        // Segment: current -> nextDot
        if (nextDot == current) return false; // Empty segment (..) or leading dot

        // Check rules for Prerelease Identifier
        // Regex: (?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*)
        if (!checkSegment(input, current, nextDot, true)) return false;

        if (nextDot == end) break; // Finished
        if (nextDot == end - 1) return false; // Trailing dot (e.g. "alpha.")
        
        current = nextDot + 1;
    }
    return true;
}

bool SemVer::validateBuild(const String& input, int start, int end) const {
    if (start >= end) return false; // Empty build specified

    int current = start;
    while (current < end) {
        int nextDot = input.indexOf('.', current);
        if (nextDot == -1 || nextDot > end) nextDot = end;

        if (nextDot == current) return false; // Empty segment

        // Build identifiers: [0-9A-Za-z-]+ (Leading zeros allowed)
        for (int i = current; i < nextDot; i++) {
            char c = input.charAt(i);
            if (!isDigit(c) && !isAlpha(c) && c != '-') return false;
        }

        if (nextDot == end) break;
        if (nextDot == end - 1) return false; // Trailing dot

        current = nextDot + 1;
    }
    return true;
}

bool SemVer::parseCore(const String& input, int dot1, int dot2, int endOfPatch, uint32_t& maj, uint32_t& min, uint32_t& pat) const {
    if (!parseUint32(input, 0, dot1, maj)) return false;
    if (!parseUint32(input, dot1 + 1, dot2, min)) return false;
    if (!parseUint32(input, dot2 + 1, endOfPatch, pat)) return false;
    return true;
}

void SemVer::commit(uint32_t maj, uint32_t min, uint32_t pat, const String& pre, const String& bld) {
    this->major = maj;
    this->minor = min;
    this->patch = pat;
    this->prerelease = pre;
    this->build = bld;
    this->valid = true;
}

// Helpers

// Helper to validate a segment based on SemVer rules
// isPrerelease = false -> Core Version Rules (Numeric only, no leading zero)
// isPrerelease = true  -> Prerelease Rules (Alphanum, numeric-only cannot have leading zero)
bool SemVer::checkSegment(const String& s, int start, int end, bool isPrerelease) const {
    if (start >= end) return false;
    
    bool isAllNumeric = true;
    for (int i = start; i < end; i++) {
        char c = s.charAt(i);
        if (!isDigit(c)) {
            isAllNumeric = false;
            if (!isPrerelease) return false; // Core must be digits only
            // For prerelease, check allowed chars [0-9A-Za-z-]
            if (!isAlpha(c) && c != '-') return false;
        }
    }

    // Leading Zero Rule
    // Applies to Core (always numeric) AND Prerelease (if identifier is numeric)
    // Rule: "0" is allowed. "01" is not.
    if (isAllNumeric) {
        if ((end - start) > 1 && s.charAt(start) == '0') {
            return false;
        }
    }

    return true;
}

bool SemVer::parseUint32(const String& s, int start, int end, uint32_t& out) const {
    // We assume validation passed, so format is correct. We check for overflow.
    // Avoid String allocation by manual parsing
    uint32_t val = 0;
    uint32_t cutoff = 429496729; // UINT32_MAX / 10
    uint32_t cutlim = 5;         // UINT32_MAX % 10

    for (int i = start; i < end; i++) {
        char c = s.charAt(i);
        uint32_t digit = c - '0';
        
        if (val > cutoff || (val == cutoff && digit > cutlim)) {
            return false; // Overflow
        }
        val = val * 10 + digit;
    }
    out = val;
    return true;
}

// Helper needed for isNumeric check within legacy/other parts of code if reused
// Refactored to use index-based helper
bool SemVer::isNumeric(const String& s, int start, int end) const {
    if (start >= end) return false;
    for (int i = start; i < end; i++) {
        if (!isDigit(s.charAt(i))) return false;
    }
    return true;
}

// Keeping the member function for compatibility if header defines it, 
// but mapping to new logic
bool SemVer::isNumeric(const String& s) const {
    return isNumeric(s, 0, s.length());
}

bool SemVer::isValid() const {
    return valid;
}

String SemVer::toString() const {
    if (!valid) return "invalid";
    String res = String(major) + "." + String(minor) + "." + String(patch);
    if (prerelease.length() > 0) res += "-" + prerelease;
    if (build.length() > 0) res += "+" + build;
    return res;
}

bool SemVer::operator==(const SemVer& other) const {
    if (!valid || !other.valid) return false;
    return major == other.major && minor == other.minor && patch == other.patch && prerelease == other.prerelease;
}

bool SemVer::operator!=(const SemVer& other) const {
    return !(*this == other);
}

bool SemVer::operator<(const SemVer& other) const {
    if (!valid || !other.valid) return false;
    if (major != other.major) return major < other.major;
    if (minor != other.minor) return minor < other.minor;
    if (patch != other.patch) return patch < other.patch;
    
    // Release version has higher precedence than pre-release
    if (prerelease.length() > 0 && other.prerelease.length() == 0) return true;
    if (prerelease.length() == 0 && other.prerelease.length() > 0) return false;
    if (prerelease.length() == 0 && other.prerelease.length() == 0) return false;
    
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
    SemVer v1(baseVersion);
    SemVer v2(newVersion);
    if (!v1.valid || !v2.valid) return false;
    return v2 > v1;
}

SemVer SemVer::coerce(const String& versionString) {
    // Simple coercion: find the first digit and try to parse from there
    int start = -1;
    for (unsigned int i = 0; i < versionString.length(); i++) {
        if (isDigit(versionString.charAt(i))) {
            start = i;
            break;
        }
    }
    if (start == -1) return SemVer();

    String s = versionString.substring(start);
    // Try full parse first
    SemVer v(s);
    if (v.valid) return v;

    // Try partial: X.Y or X
    int dot1 = s.indexOf('.');
    if (dot1 == -1) {
        // Just a major version?
        // Check if it's purely numeric
        int end = 0;
        while(end < (int)s.length() && isDigit(s.charAt(end))) end++;
        if (end == 0) return SemVer();
        SemVer res;
        if (res.parseUint32(s, 0, end, res.major)) {
            res.minor = 0;
            res.patch = 0;
            res.valid = true;
            return res;
        }
        return SemVer();
    }

    int dot2 = s.indexOf('.', dot1 + 1);
    if (dot2 == -1) {
        // X.Y?
        int end = dot1 + 1;
        while(end < (int)s.length() && isDigit(s.charAt(end))) end++;
        SemVer res;
        if (res.parseUint32(s, 0, dot1, res.major) && res.parseUint32(s, dot1+1, end, res.minor)) {
            res.patch = 0;
            res.valid = true;
            
            // Check for trailing prerelease/build
            int hyphen = s.indexOf('-', end);
            int plus = s.indexOf('+', end);
            if (hyphen != -1 || plus != -1) {
                // Too complex for simple coerce if not standard
                // But let's try to see if we can just re-parse it as X.Y.0...
                String full = s.substring(0, dot1) + "." + s.substring(dot1 + 1, end) + ".0" + s.substring(end);
                return SemVer(full);
            }
            return res;
        }
    }

    return SemVer(); // Fallback
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

// IMPORTANT: Updated comparePrerelease to use index-based substrings or correct logic
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

        if (partA.length() == 0 && partB.length() > 0) return -1; 
        if (partA.length() > 0 && partB.length() == 0) return 1;

        bool aNum = isNumeric(partA, 0, partA.length());
        bool bNum = isNumeric(partB, 0, partB.length());

        if (aNum && bNum) {
            if (partA.length() != partB.length()) {
                return (partA.length() < partB.length()) ? -1 : 1;
            }
            int cmp = partA.compareTo(partB);
            if (cmp != 0) return cmp;
        } else if (aNum && !bNum) {
            return -1; 
        } else if (!aNum && bNum) {
            return 1; 
        } else {
            int cmp = partA.compareTo(partB);
            if (cmp != 0) return cmp;
        }

        startA = endA + 1;
        startB = endB + 1;
    }
    return 0;
}