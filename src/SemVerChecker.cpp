#include "SemVerChecker.h"


size_t SemVer::custom_strlen(const char* s) {
    if (!s) return 0;
    size_t len = 0;
    while (s[len]) len++;
    return len;
}

size_t SemVer::custom_strnlen(const char* s, size_t max_len) {
    if (!s) return 0;
    size_t len = 0;
    while (len < max_len && s[len]) len++;
    return len;
}

int SemVer::custom_strcmp(const char* s1, const char* s2) {
    if (!s1 || !s2) return (s1 == s2) ? 0 : (s1 ? 1 : -1);
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

char* SemVer::custom_strncpy(char* dest, const char* src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++)
        dest[i] = src[i];
    for (; i < n; i++)
        dest[i] = '\0';
    return dest;
}

int SemVer::findChar(const char* s, char c, int start) {
    if (!s) return -1;
    for (int i = start; s[i] != '\0'; i++) {
        if (s[i] == c) return i;
    }
    return -1;
}


SemVer::SemVer() : major(0), minor(0), patch(0), _preOffset(0), _buildOffset(0), _valid(false) {
    _buffer[0] = '\0';
}

SemVer::SemVer(const char* versionString) : major(0), minor(0), patch(0), _preOffset(0), _buildOffset(0), _valid(false) {
    parse(versionString);
}

#ifdef ARDUINO
SemVer::SemVer(const String& versionString) : major(0), minor(0), patch(0), _preOffset(0), _buildOffset(0), _valid(false) {
    parse(versionString.c_str());
}
#endif

void SemVer::parse(const char* input) {
    if (!basicGuards(input)) {
        _valid = false;
        return;
    }

    // Copy to internal buffer
    custom_strncpy(_buffer, input, MAX_VERSION_LEN);
    _buffer[MAX_VERSION_LEN] = '\0';

    int dot1, dot2, hyphenIndex, plusIndex;
    if (!splitMainParts(_buffer, dot1, dot2, hyphenIndex, plusIndex)) {
        _valid = false;
        return;
    }

    int endOfPatch = (hyphenIndex != -1) ? hyphenIndex : ((plusIndex != -1) ? plusIndex : (int)custom_strlen(_buffer));
    int startOfPre = (hyphenIndex != -1) ? hyphenIndex + 1 : -1;
    int endOfPre   = (plusIndex != -1) ? plusIndex : (int)custom_strlen(_buffer);
    int startOfBuild = (plusIndex != -1) ? plusIndex + 1 : -1;
    int endOfBuild = (int)custom_strlen(_buffer);

    if (!validateCore(_buffer, dot1, dot2, endOfPatch)) {
        _valid = false;
        return;
    }

    if (startOfPre != -1) {
        if (!validatePrerelease(_buffer, startOfPre, endOfPre)) {
            _valid = false;
            return;
        }
    }

    if (startOfBuild != -1) {
        if (!validateBuild(_buffer, startOfBuild, endOfBuild)) {
            _valid = false;
            return;
        }
    }

    uint32_t tmpMajor, tmpMinor, tmpPatch;
    if (!parseCore(_buffer, dot1, dot2, endOfPatch, tmpMajor, tmpMinor, tmpPatch)) {
        _valid = false;
        return;
    }

    // Commit values
    major = tmpMajor;
    minor = tmpMinor;
    patch = tmpPatch;
    _preOffset = (startOfPre != -1) ? (uint16_t)startOfPre : 0;
    _buildOffset = (startOfBuild != -1) ? (uint16_t)startOfBuild : 0;
    _valid = true;

    // In-place terminate segments with \0 to allow direct pointer use
    if (hyphenIndex != -1) _buffer[hyphenIndex] = '\0';
    if (plusIndex != -1) _buffer[plusIndex] = '\0';
}

bool SemVer::basicGuards(const char* input) const {
    if (!input || input[0] == '\0') return false;
    // Check up to MAX_VERSION_LEN + 1. If longer, reject.
    size_t len = custom_strnlen(input, MAX_VERSION_LEN + 1);
    if (len > MAX_VERSION_LEN) return false;
    return true;
}

bool SemVer::splitMainParts(const char* input, int& dot1, int& dot2, int& hyphen, int& plus) const {
    dot1 = findChar(input, '.');
    if (dot1 == -1) return false;

    dot2 = findChar(input, '.', dot1 + 1);
    if (dot2 == -1) return false;

    plus = findChar(input, '+');
    
    int scanStart = dot2 + 1;
    hyphen = -1;
    
    for (int i = scanStart; input[i] != '\0'; i++) {
        char c = input[i];
        if (c == '+') break; 
        if (c == '-') {
            hyphen = i;
            break;
        }
        if (c < '0' || c > '9') return false;
    }
    
    if (hyphen != -1 && plus != -1 && hyphen > plus) return false;
    if (dot1 == 0 || (dot2 - dot1) <= 1) return false;
    
    return true;
}

bool SemVer::validateCore(const char* input, int dot1, int dot2, int endOfPatch) const {
    if (!checkSegment(input, 0, dot1, false)) return false;
    if (!checkSegment(input, dot1 + 1, dot2, false)) return false;
    if (!checkSegment(input, dot2 + 1, endOfPatch, false)) return false;
    return true;
}

bool SemVer::validatePrerelease(const char* input, int start, int end) const {
    if (start >= end) return false;

    int current = start;
    while (current < end) {
        int nextDot = findChar(input, '.', current);
        if (nextDot == -1 || nextDot > end) nextDot = end;
        
        if (nextDot == current) return false;
        if (!checkSegment(input, current, nextDot, true)) return false;
        if (nextDot == end) break;
        if (nextDot == end - 1) return false; 
        current = nextDot + 1;
    }
    return true;
}

bool SemVer::validateBuild(const char* input, int start, int end) const {
    if (start >= end) return false;

    int current = start;
    while (current < end) {
        int nextDot = findChar(input, '.', current);
        if (nextDot == -1 || nextDot > end) nextDot = end;

        if (nextDot == current) return false;

        for (int i = current; i < nextDot; i++) {
            char c = input[i];
            bool isAlpha = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
            bool isDigit = (c >= '0' && c <= '9');
            if (!isDigit && !isAlpha && c != '-') return false;
        }

        if (nextDot == end) break;
        if (nextDot == end - 1) return false;

        current = nextDot + 1;
    }
    return true;
}

bool SemVer::parseCore(const char* input, int dot1, int dot2, int endOfPatch, uint32_t& maj, uint32_t& min, uint32_t& pat) const {
    if (!parseUint32(input, 0, dot1, maj)) return false;
    if (!parseUint32(input, dot1 + 1, dot2, min)) return false;
    if (!parseUint32(input, dot2 + 1, endOfPatch, pat)) return false;
    return true;
}

bool SemVer::checkSegment(const char* s, int start, int end, bool isPrerelease) const {
    if (start >= end) return false;
    
    bool isAllNumeric = true;
    for (int i = start; i < end; i++) {
        char c = s[i];
        bool isDigit = (c >= '0' && c <= '9');
        if (!isDigit) {
            isAllNumeric = false;
            if (!isPrerelease) return false;
            bool isAlpha = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
            if (!isAlpha && c != '-') return false;
        }
    }

    if (isAllNumeric) {
        if ((end - start) > 1 && s[start] == '0') return false;
    }
    return true;
}

bool SemVer::parseUint32(const char* s, int start, int end, uint32_t& out) const {
    uint32_t val = 0;
    uint32_t cutoff = 429496729;
    uint32_t cutlim = 5;

    for (int i = start; i < end; i++) {
        uint32_t digit = s[i] - '0';
        if (val > cutoff || (val == cutoff && digit > cutlim)) return false;
        val = val * 10 + digit;
    }
    out = val;
    return true;
}

bool SemVer::isNumeric(const char* s, int start, int end) const {
    if (start >= end) return false;
    for (int i = start; i < end; i++) {
        if (s[i] < '0' || s[i] > '9') return false;
    }
    return true;
}

bool SemVer::isValid() const {
    return _valid;
}

const char* SemVer::getPrerelease() const {
    return _preOffset ? &_buffer[_preOffset] : "";
}

const char* SemVer::getBuild() const {
    return _buildOffset ? &_buffer[_buildOffset] : "";
}

void SemVer::toString(char* buffer, size_t len) const {
    if (!buffer || len == 0) return;
    if (!_valid) {
        custom_strncpy(buffer, "invalid", len);
        buffer[len - 1] = '\0';
        return;
    }


    auto writeNum = [](char* b, size_t& pos, size_t l, uint32_t n) {
        if (pos >= l) return;
        if (n == 0) {
            if (pos < l - 1) b[pos++] = '0';
            return;
        }
        char temp[11];
        int i = 0;
        while (n > 0) {
            temp[i++] = (n % 10) + '0';
            n /= 10;
        }
        while (i > 0 && pos < l - 1) {
            b[pos++] = temp[--i];
        }
    };

    size_t pos = 0;
    writeNum(buffer, pos, len, major);
    if (pos < len - 1) buffer[pos++] = '.';
    writeNum(buffer, pos, len, minor);
    if (pos < len - 1) buffer[pos++] = '.';
    writeNum(buffer, pos, len, patch);

    const char* pre = getPrerelease();
    if (pre[0] != '\0' && pos < len - 1) {
        buffer[pos++] = '-';
        while (*pre && pos < len - 1) buffer[pos++] = *pre++;
    }

    const char* bld = getBuild();
    if (bld[0] != '\0' && pos < len - 1) {
        buffer[pos++] = '+';
        while (*bld && pos < len - 1) buffer[pos++] = *bld++;
    }

    if (pos < len) buffer[pos] = '\0';
    else buffer[len - 1] = '\0';
}

#ifdef ARDUINO
String SemVer::toString() const {
    char buf[MAX_VERSION_LEN + 1];
    toString(buf, sizeof(buf));
    return String(buf);
}

size_t SemVer::printTo(Print& p) const {
    if (!_valid) {
        return p.print("invalid");
    }

    // Write version components directly to Print stream
    // This avoids creating temporary String objects
    size_t n = 0;

    auto writeNum = [&p, &n](uint32_t num) {
        if (num == 0) {
            n += p.print('0');
            return;
        }
        char temp[11]; // max uint32 is 10 digits
        int i = 0;
        while (num > 0) {
            temp[i++] = (num % 10) + '0';
            num /= 10;
        }
        // Write in reverse order
        while (i > 0) {
            n += p.print(temp[--i]);
        }
    };

    writeNum(major);
    n += p.print('.');
    writeNum(minor);
    n += p.print('.');
    writeNum(patch);

    const char* pre = getPrerelease();
    if (pre[0] != '\0') {
        n += p.print('-');
        n += p.print(pre);
    }

    const char* bld = getBuild();
    if (bld[0] != '\0') {
        n += p.print('+');
        n += p.print(bld);
    }

    return n;
}
#endif


bool SemVer::operator==(const SemVer& other) const {
    if (!_valid || !other._valid) return false;
    return major == other.major && minor == other.minor && patch == other.patch && 
           custom_strcmp(getPrerelease(), other.getPrerelease()) == 0;
}

bool SemVer::operator!=(const SemVer& other) const {
    return !(*this == other);
}

bool SemVer::operator<(const SemVer& other) const {
    if (!_valid || !other._valid) return false;
    if (major != other.major) return major < other.major;
    if (minor != other.minor) return minor < other.minor;
    if (patch != other.patch) return patch < other.patch;
    
    const char* preA = getPrerelease();
    const char* preB = other.getPrerelease();
    bool hasPreA = (preA[0] != '\0');
    bool hasPreB = (preB[0] != '\0');

    if (hasPreA && !hasPreB) return true;
    if (!hasPreA && hasPreB) return false;
    if (!hasPreA && !hasPreB) return false;
    
    return comparePrerelease(preA, preB) < 0;
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

bool SemVer::isUpgrade(const char* baseVersion, const char* newVersion) {
    SemVer v1(baseVersion);
    SemVer v2(newVersion);
    return v1._valid && v2._valid && v2 > v1;
}

#ifdef ARDUINO
bool SemVer::isUpgrade(const String& baseVersion, const String& newVersion) {
    return isUpgrade(baseVersion.c_str(), newVersion.c_str());
}
#endif

bool SemVer::satisfies(const SemVer& requirement) const {
    // Both versions must be valid
    if (!_valid || !requirement._valid) {
        return false;
    }

    // Version must be >= requirement
    if (*this < requirement) {
        return false;
    }

    // Major version must match (SemVer breaking change rule)
    if (major != requirement.major) {
        return false;
    }

    // Special case for 0.x.x: minor version changes are also breaking
    // Per SemVer spec: "Major version zero (0.y.z) is for initial development.
    // Anything MAY change at any time. The public API SHOULD NOT be considered stable."
    if (major == 0 && minor != requirement.minor) {
        return false;
    }

    return true;
}

SemVer SemVer::maximum(const SemVer& v1, const SemVer& v2) {
    // If both invalid, return invalid
    if (!v1._valid && !v2._valid) {
        return SemVer();
    }
    
    // If one is invalid, return the valid one
    if (!v1._valid) return v2;
    if (!v2._valid) return v1;
    
    // Both valid, return the greater one
    return (v1 > v2) ? v1 : v2;
}

SemVer SemVer::minimum(const SemVer& v1, const SemVer& v2) {
    // If both invalid, return invalid
    if (!v1._valid && !v2._valid) {
        return SemVer();
    }
    
    // If one is invalid, return the valid one
    if (!v1._valid) return v2;
    if (!v2._valid) return v1;
    
    // Both valid, return the lesser one
    return (v1 < v2) ? v1 : v2;
}





SemVer::DiffType SemVer::diff(const SemVer& other) const {
    if (!_valid || !other._valid) return NONE;
    if (major != other.major) return MAJOR;
    if (minor != other.minor) return MINOR;
    if (patch != other.patch) return PATCH;
    if (custom_strcmp(getPrerelease(), other.getPrerelease()) != 0) return PRERELEASE;
    return NONE;
}

void SemVer::incMajor() {
    major++; minor = 0; patch = 0;
    _preOffset = 0; _buildOffset = 0;
    _buffer[0] = '\0';
}

void SemVer::incMinor() {
    minor++; patch = 0;
    _preOffset = 0; _buildOffset = 0;
    _buffer[0] = '\0';
}

void SemVer::incPatch() {
    patch++;
    _preOffset = 0; _buildOffset = 0;
    _buffer[0] = '\0';
}

int SemVer::comparePrerelease(const char* a, const char* b) const {
    if (custom_strcmp(a, b) == 0) return 0;
    
    int startA = 0, startB = 0;
    size_t lenA = custom_strlen(a);
    size_t lenB = custom_strlen(b);
    
    while (startA < (int)lenA || startB < (int)lenB) {
        int endA = findChar(a, '.', startA);
        int endB = findChar(b, '.', startB);
        if (endA == -1) endA = (int)lenA;
        if (endB == -1) endB = (int)lenB;

        if (startA >= (int)lenA && startB < (int)lenB) return -1;
        if (startA < (int)lenA && startB >= (int)lenB) return 1;

        int segLenA = endA - startA;
        int segLenB = endB - startB;

        bool aNum = isNumeric(a, startA, endA);
        bool bNum = isNumeric(b, startB, endB);

        if (aNum && bNum) {
            if (segLenA != segLenB) return (segLenA < segLenB) ? -1 : 1;
            for (int i = 0; i < segLenA; i++) {
                if (a[startA + i] != b[startB + i]) return (a[startA + i] < b[startB + i]) ? -1 : 1;
            }
        } else if (aNum && !bNum) {
            return -1; 
        } else if (!aNum && bNum) {
            return 1; 
        } else {
            int minLen = (segLenA < segLenB) ? segLenA : segLenB;
            for (int i = 0; i < minLen; i++) {
                if (a[startA + i] != b[startB + i]) return (a[startA + i] < b[startB + i]) ? -1 : 1;
            }
            if (segLenA != segLenB) return (segLenA < segLenB) ? -1 : 1;
        }

        startA = (endA < (int)lenA) ? endA + 1 : (int)lenA;
        startB = (endB < (int)lenB) ? endB + 1 : (int)lenB;
    }
    return 0;
}