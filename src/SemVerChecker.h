#ifndef SEMVERCHECKER_H
#define SEMVERCHECKER_H

#include <Arduino.h>
#include <stdint.h>

class SemVer {
public:
    uint32_t major;
    uint32_t minor;
    uint32_t patch;
    String prerelease;
    String build;
    bool valid;

    static const size_t MAX_VERSION_LEN = 128;

    SemVer();
    SemVer(String versionString);
    SemVer(const char* versionString);

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

    static bool isUpgrade(String baseVersion, String newVersion);
    static SemVer coerce(String versionString);

    DiffType diff(const SemVer& other) const;
    void incMajor();
    void incMinor();
    void incPatch();

private:
    void parse(String versionString);
    int comparePrerelease(const String& a, const String& b) const;
    bool isNumeric(const String& s, bool allowLeadingZeros = false) const;
    bool parseUint32(const String& s, uint32_t& out) const;
    bool isValidChar(char c, bool allowDot = false) const;
};

#endif
