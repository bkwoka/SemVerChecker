#ifndef ARDUINO_H
#define ARDUINO_H

#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>
#include <vector>

// Mock for Arduino String class
class String {
public:
    std::string _s;

    String() : _s("") {}
    String(const char* s) : _s(s ? s : "") {}
    String(const std::string& s) : _s(s) {}
    String(int i) : _s(std::to_string(i)) {}
    String(long l) : _s(std::to_string(l)) {}

    size_t length() const {
        return _s.length();
    }

    int indexOf(char c) const {
        size_t pos = _s.find(c);
        return (pos == std::string::npos) ? -1 : (int)pos;
    }

    int indexOf(char c, size_t fromIndex) const {
        if (fromIndex >= _s.length()) return -1;
        size_t pos = _s.find(c, fromIndex);
        return (pos == std::string::npos) ? -1 : (int)pos;
    }

    int indexOf(const char* s, size_t fromIndex = 0) const {
        if (fromIndex >= _s.length()) return -1;
        size_t pos = _s.find(s, fromIndex);
        return (pos == std::string::npos) ? -1 : (int)pos;
    }

    String substring(int start) const {
        if (start >= _s.length()) return String("");
        return String(_s.substr(start));
    }

    String substring(int start, int end) const {
        if (start >= _s.length()) return String("");
        if (end > _s.length()) end = _s.length();
        if (start > end) return String("");
        return String(_s.substr(start, end - start));
    }

    long toInt() const {
        try {
            return std::stol(_s);
        } catch (...) {
            return 0;
        }
    }

    char charAt(size_t index) const {
        if (index >= _s.length()) return 0;
        return _s[index];
    }

    bool startsWith(const String& prefix) const {
        if ((unsigned int)prefix.length() > (unsigned int)length()) return false;
        return _s.compare(0, prefix.length(), prefix._s) == 0;
    }

    bool endsWith(const String& suffix) const {
        if ((unsigned int)suffix.length() > (unsigned int)length()) return false;
        return _s.compare(length() - suffix.length(), suffix.length(), suffix._s) == 0;
    }

    int compareTo(const String& other) const {
        return _s.compare(other._s);
    }
    
    // Operators
    bool operator==(const String& other) const {
        return _s == other._s;
    }

    bool operator!=(const String& other) const {
        return _s != other._s;
    }
    
    bool operator<(const String& other) const {
        return _s < other._s;
    }

    String operator+(const String& other) const {
        return String(_s + other._s);
    }

    String operator+(const char* other) const {
        return String(_s + other);
    }
    
    friend std::ostream& operator<<(std::ostream& os, const String& s) {
        os << s._s;
        return os;
    }

    String& operator+=(const String& other) {
        _s += other._s;
        return *this;
    }

    String& operator+=(const char* other) {
        _s += other;
        return *this;
    }
};

// Global operators
inline String operator+(const char* lhs, const String& rhs) {
    return String(lhs) + rhs;
}
// Global helper functions
inline bool isDigit(char c) {
    return std::isdigit(static_cast<unsigned char>(c));
}

#endif
