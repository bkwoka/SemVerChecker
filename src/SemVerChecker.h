#ifndef SEMVERCHECKER_H
#define SEMVERCHECKER_H

#include <Arduino.h>

class SemVer {
public:
    int major;
    int minor;
    int patch;
    String prerelease;
    String build;
    bool valid;

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
    bool isNumeric(const String& s) const;
};

#endif
