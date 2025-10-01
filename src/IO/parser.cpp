#include <charconv>   // for std::from_chars (C++17+)
#include <cstdlib>    // for strtof (fallback)
#include <cstring>

#include "IO/parser.h"

// Skip whitespace
void Parser::SkipSpaces() {
    while (*m_sv == ' ' || *m_sv == '\t') ++m_sv;
}

int Parser::TakeIndex(int baseCount) {
    if (!m_sv || *m_sv == '\0') return -1;

    bool neg = (*m_sv == '-');
    if (neg) ++m_sv;

    int idx = 0;
    while (*m_sv >= '0' && *m_sv <= '9') {
        idx = idx * 10 + (*m_sv - '0');
        ++m_sv;
    }

    if (neg) return baseCount + (-idx);
    return idx > 0 ? idx - 1 : -1;
}

// Get next whitespace-delimited word
char* Parser::TakeWord() {
    SkipSpaces();
    if (*m_sv == '\0' || *m_sv == '\n' || *m_sv == '\r') return nullptr;

    char* start = m_sv;
    while (*m_sv && *m_sv != ' ' && *m_sv != '\t' && *m_sv != '\n' && *m_sv != '\r')
        ++m_sv;

    if (*m_sv) { *m_sv = '\0'; ++m_sv; }
    return start;
}

// Parse a float quickly
float Parser::TakeFloat() {
    SkipSpaces();
    if (*m_sv == '\0') return 0.0f;

#if __cpp_lib_to_chars >= 201611L
    float value = 0.0f;
    auto result = std::from_chars(m_sv, m_sv + std::strlen(m_sv), value);
    m_sv = const_cast<char*>(result.ptr);
    return value;
#else
    char* end;
    float value = std::strtof(m_sv, &end);
    m_sv = end;
    return value;
#endif
}

// Parse an integer quickly
int Parser::TakeInt() {
    SkipSpaces();
    if (*m_sv == '\0') return 0;

#if __cpp_lib_to_chars >= 201611L
    int value = 0;
    auto result = std::from_chars(m_sv, m_sv + std::strlen(m_sv), value);
    m_sv = const_cast<char*>(result.ptr);
    return value;
#else
    char* end;
    int value = static_cast<int>(std::strtol(m_sv, &end, 10));
    m_sv = end;
    return value;
#endif
}

// Take everything until delimiter (mutates buffer)
char* Parser::TakeUntil(char d) {
    SkipSpaces();
    if (*m_sv == '\0') return nullptr;

    char* start = m_sv;
    while (*m_sv && *m_sv != d && *m_sv != '\n' && *m_sv != '\r')
        ++m_sv;

    if (*m_sv) { *m_sv = '\0'; ++m_sv; }
    return start;
}

// Parser.h (or Parser.cpp)
// Parse one face element at current position.
// Accepts formats: "v", "v/t", "v//n", "v/t/n"
// Returns true if a token was parsed, false if no more tokens on the line.
bool Parser::TakeFaceIndices(int &vi, int &ti, int &ni) {
    SkipSpaces();
    if (*m_sv == '\0' || *m_sv == '\n' || *m_sv == '\r') {
        vi = ti = ni = 0; // sentinel raw value meaning "no token"
        return false;
    }

    // parse vertex index (may be negative)
    vi = static_cast<int>(std::strtol(m_sv, &m_sv, 10));

    ti = ni = 0; // 0 = not present (raw)
    if (*m_sv == '/') {
        ++m_sv; // skip '/'
        // texcoord index (optional)
        if (*m_sv != '/' && *m_sv != ' ' && *m_sv != '\0' && *m_sv != '\n' && *m_sv != '\r') {
            ti = static_cast<int>(std::strtol(m_sv, &m_sv, 10));
        }
        if (*m_sv == '/') {
            ++m_sv; // skip second '/'
            // normal index (optional)
            if (*m_sv != ' ' && *m_sv != '\0' && *m_sv != '\n' && *m_sv != '\r') {
                ni = static_cast<int>(std::strtol(m_sv, &m_sv, 10));
            }
        }
    }

    // At this point m_sv is either at whitespace, end, or next token char.
    // Do NOT mutate indices (leave them raw). Let NormalizeIndex handle conversion.
    return true;
}