#ifndef SEMVERCHECKER_H
#define SEMVERCHECKER_H

#include <stddef.h>
#include <stdint.h>

#ifdef ARDUINO
#include <Arduino.h>
#endif

class SemVer {
public:
    // Use uint32_t per SemVer spec (non-negative integers)
    uint32_t major;
    uint32_t minor;
    uint32_t patch;

    // Length limit for security (protection against DoS/memory exhaustion)
    static const size_t MAX_VERSION_LEN = 64; 

    SemVer();
    explicit SemVer(const char* versionString);
#ifdef ARDUINO
    explicit SemVer(const String& versionString);
#endif

    bool isValid() const;
    
    // String output
    void toString(char* buffer, size_t len) const;
#ifdef ARDUINO
    String toString() const;
#endif

    // Getters for metadata (returns pointers to internal buffer)
    const char* getPrerelease() const;
    const char* getBuild() const;

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

    static bool isUpgrade(const char* baseVersion, const char* newVersion);
#ifdef ARDUINO
    static bool isUpgrade(const String& baseVersion, const String& newVersion);
#endif

    /** 
     * @brief Coerces a string into a SemVer object.
     * @warning This function uses naive heuristic logic and may behave unpredictably with complex or malformed input.
     */
    static SemVer coerce(const char* versionString);
#ifdef ARDUINO
    /** 
     * @brief Coerces a string into a SemVer object.
     * @warning This function uses naive heuristic logic and may behave unpredictably with complex or malformed input.
     */
    static SemVer coerce(const String& versionString);
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

    // Architecture methods
    void parse(const char* input);
    bool basicGuards(const char* input) const;
    bool splitMainParts(const char* input, int& dot1, int& dot2, int& hyphen, int& plus) const;
    bool validateCore(const char* input, int dot1, int dot2, int endOfPatch) const;
    bool validatePrerelease(const char* input, int start, int end) const;
    bool validateBuild(const char* input, int start, int end) const;
    bool parseCore(const char* input, int dot1, int dot2, int endOfPatch, uint32_t& maj, uint32_t& min, uint32_t& pat) const;

    // Helpers
    int comparePrerelease(const char* a, const char* b) const;
    bool isNumeric(const char* s, int start, int end) const;
    bool checkSegment(const char* s, int start, int end, bool isPrerelease) const;
    bool parseUint32(const char* s, int start, int end, uint32_t& out) const;
    
    // Internal utils
    static size_t custom_strlen(const char* s);
    static int custom_strcmp(const char* s1, const char* s2);
    static char* custom_strncpy(char* dest, const char* src, size_t n);
    static int findChar(const char* s, char c, int start = 0);
};

#endif