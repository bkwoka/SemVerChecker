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
    
    void incMajor();
    void incMinor();
    void incPatch();

private:
    // Architecture methods
    void parse(const String& inputRaw);
    bool basicGuards(const String& input) const;
    bool splitMainParts(const String& input, int& dot1, int& dot2, int& hyphen, int& plus) const;
    bool validateCore(const String& input, int dot1, int dot2, int endOfPatch) const;
    bool validatePrerelease(const String& input, int start, int end) const;
    bool validateBuild(const String& input, int start, int end) const;
    bool parseCore(const String& input, int dot1, int dot2, int endOfPatch, uint32_t& maj, uint32_t& min, uint32_t& pat) const;
    void commit(uint32_t maj, uint32_t min, uint32_t pat, const String& pre, const String& bld);

    // Helpers
    int comparePrerelease(const String& a, const String& b) const;
    bool isNumeric(const String& s) const;
    bool isNumeric(const String& s, int start, int end) const;
    bool checkSegment(const String& s, int start, int end, bool isPrerelease) const;
    bool parseUint32(const String& s, int start, int end, uint32_t& out) const;
};

#endif