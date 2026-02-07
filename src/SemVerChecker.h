#ifndef SEMVERCHECKER_H
#define SEMVERCHECKER_H

#include <Arduino.h>
#include <stdint.h>

class SemVer {
public:
    // Use uint32_t per SemVer spec (non-negative integers)
    uint32_t major;
    uint32_t minor;
    uint32_t patch;
    String prerelease;
    String build;
    bool valid;

    // Length limit for security (protection against DoS/memory exhaustion)
    static const size_t MAX_VERSION_LEN = 64; 

    SemVer();
    // Pass-by-reference to avoid heap allocations
    explicit SemVer(const String& versionString); 
    explicit SemVer(const char* versionString);

    bool isValid() const;
    String toString() const;

    // Comparison operators
    bool operator==(const SemVer& other) const;
    bool operator!=(const SemVer& other) const;
    bool operator<(const SemVer& other) const;
    bool operator>(const SemVer& other) const;
    bool operator<=(const SemVer& other) const;
    bool operator>=(const SemVer& other) const;
    
    enum DiffType {
        NONE,
        MAJOR,
        MINOR,
        PATCH,
        PRERELEASE
    };

    static bool isUpgrade(const String& baseVersion, const String& newVersion);
    static SemVer coerce(const String& versionString);

    DiffType diff(const SemVer& other) const;
    
    // Modification methods (resets build/prerelease)
    void incMajor();
    void incMinor();
    void incPatch();

private:
    void parse(const String& versionString);
    int comparePrerelease(const String& a, const String& b) const;
    
    // Helpers
    bool isNumeric(const String& s) const;
    bool parseUint32(const String& s, uint32_t& out) const;
    bool isValidIdentifier(const String& fullId, bool allowLeadingZeros) const;
};

#endif
