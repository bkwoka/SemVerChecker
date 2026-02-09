#ifndef SEMVERCHECKER_H
#define SEMVERCHECKER_H

#include <stddef.h>
#include <stdint.h>

#ifdef ARDUINO
#include <Arduino.h>
#endif

#ifdef ARDUINO
class SemVer : public Printable {
#else
class SemVer {
#endif
public:
    // Use uint32_t per SemVer spec (non-negative integers)
    uint32_t major;
    uint32_t minor;
    uint32_t patch;

    // Length limit for security (protection against DoS/memory exhaustion)
    // Can be configured via build flags: -DSEMVER_MAX_LENGTH=128
#ifndef SEMVER_MAX_LENGTH
#define SEMVER_MAX_LENGTH 64
#endif
    static const size_t MAX_VERSION_LEN = SEMVER_MAX_LENGTH; 

    SemVer();
    explicit SemVer(const char* versionString);
#ifdef ARDUINO
    explicit SemVer(const String& versionString);
#endif

    bool isValid() const;
    

    void toString(char* buffer, size_t len) const;
#ifdef ARDUINO
    String toString() const;
    
    /**
     * @brief Printable interface implementation for Arduino Serial output
     * @param p Print object (e.g., Serial)
     * @return Number of bytes written
     */
    virtual size_t printTo(Print& p) const;
#endif
    
    /**
     * @brief Check if this version satisfies the requirement (Caret range logic)
     * @param requirement The required version to check against
     * @return true if this version is compatible with requirement
     * @note Returns false if major differs, or if major==0 and minor differs
     */
    bool satisfies(const SemVer& requirement) const;
    
    /**
     * @brief Returns the maximum of two versions
     * @param v1 First version
     * @param v2 Second version
     * @return The larger version, or the valid one if only one is valid
     */
    static SemVer maximum(const SemVer& v1, const SemVer& v2);
    
    /**
     * @brief Returns the minimum of two versions
     * @param v1 First version
     * @param v2 Second version
     * @return The smaller version, or the valid one if only one is valid
     */
    static SemVer minimum(const SemVer& v1, const SemVer& v2);

    // Getters (returns pointers to internal buffer)
    const char* getPrerelease() const;
    const char* getBuild() const;


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

    static bool isUpgrade(const char* baseVersion, const char* newVersion);
#ifdef ARDUINO
    static bool isUpgrade(const String& baseVersion, const String& newVersion);
#endif



    DiffType diff(const SemVer& other) const;
    
    void incMajor();
    void incMinor();
    void incPatch();

private:
    char _buffer[MAX_VERSION_LEN + 1];
    uint16_t _preOffset;   // Offset in _buffer, 0 if empty
    uint16_t _buildOffset; // Offset in _buffer, 0 if empty
    bool _valid;


    void parse(const char* input);
    bool basicGuards(const char* input) const;
    bool splitMainParts(const char* input, int& dot1, int& dot2, int& hyphen, int& plus) const;
    bool validateCore(const char* input, int dot1, int dot2, int endOfPatch) const;
    bool validatePrerelease(const char* input, int start, int end) const;
    bool validateBuild(const char* input, int start, int end) const;
    bool parseCore(const char* input, int dot1, int dot2, int endOfPatch, uint32_t& maj, uint32_t& min, uint32_t& pat) const;


    int comparePrerelease(const char* a, const char* b) const;
    bool isNumeric(const char* s, int start, int end) const;
    bool checkSegment(const char* s, int start, int end, bool isPrerelease) const;
    bool parseUint32(const char* s, int start, int end, uint32_t& out) const;
    

    static size_t custom_strlen(const char* s);
    static size_t custom_strnlen(const char* s, size_t max_len);
    static int custom_strcmp(const char* s1, const char* s2);
    static char* custom_strncpy(char* dest, const char* src, size_t n);
    static int findChar(const char* s, char c, int start = 0);
};

#endif