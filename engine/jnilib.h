

#include <string>
#include <cstdarg>
#include <memory>

/**
 * The following code comes from Android
 */

static inline uint16_t GetTrailingUtf16Char(uint32_t maybe_pair) {
    return static_cast<uint16_t>(maybe_pair >> 16);
}

static inline uint16_t GetLeadingUtf16Char(uint32_t maybe_pair) {
    return static_cast<uint16_t>(maybe_pair & 0xFFFF);
}

static inline uint32_t GetUtf16FromUtf8(const char** utf8_data_in) {
    const uint8_t one = *(*utf8_data_in)++;
    if ((one & 0x80) == 0) {
        // one-byte encoding
         return one;
    }

    const uint8_t two = *(*utf8_data_in)++;
    if ((one & 0x20) == 0) {
        // two-byte encoding
        return ((one & 0x1f) << 6) | (two & 0x3f);
    }

    const uint8_t three = *(*utf8_data_in)++;
    if ((one & 0x10) == 0) {
        return ((one & 0x0f) << 12) | ((two & 0x3f) << 6) | (three & 0x3f);
    }

    // Four byte encodings need special handling. We'll have
    // to convert them into a surrogate pair.
    const uint8_t four = *(*utf8_data_in)++;

    // Since this is a 4 byte UTF-8 sequence, it will lie between
    // U+10000 and U+1FFFFF.
    //
    // TODO: What do we do about values in (U+10FFFF, U+1FFFFF) ? The
    // spec says they're invalid but nobody appears to check for them.
    const uint32_t code_point = ((one & 0x0f) << 18) | ((two & 0x3f) << 12)
         | ((three & 0x3f) << 6) | (four & 0x3f);

    uint32_t surrogate_pair = 0;
    // Step two: Write out the high (leading) surrogate to the bottom 16 bits
    // of the of the 32 bit type.
    surrogate_pair |= ((code_point >> 10) + 0xd7c0) & 0xffff;
    // Step three : Write out the low (trailing) surrogate to the top 16 bits.
    surrogate_pair |= ((code_point & 0x03ff) + 0xdc00) << 16;

    return surrogate_pair;
}

// See http://java.sun.com/j2se/1.5.0/docs/guide/jni/spec/design.html#wp615 for the full rules.
static std::string MangleForJni(const std::string& s) noexcept {
    std::string result;
    const char* cp = &s[0];
    char buff[64];

    for (uint32_t ch = GetUtf16FromUtf8(&cp); ch; ch = GetUtf16FromUtf8(&cp)) {
        if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9')) {
            result.push_back((char) ch);
        } else if (ch == '.' || ch == '/') {
            result += "_";
        } else if (ch == '_') {
            result += "_1";
        } else if (ch == ';') {
            result += "_2";
        } else if (ch == '[') {
            result += "_3";
        } else {
            const uint16_t leading = GetLeadingUtf16Char(ch);
            const uint32_t trailing = GetTrailingUtf16Char(ch);

            snprintf(buff, sizeof(buff), "_0%04x", leading);
            result.append(buff);
            if (trailing != 0) {
                snprintf(buff, sizeof(buff), "_0%04x", leading);
                result.append(buff);
            }
        }
    }
    return result;
}


static std::string jni_short_name(const char *class_name,
                                  const char *method_name)
{
    std::string short_name;
    short_name += "Java_";
    short_name += MangleForJni(class_name);
    short_name += "_";
    short_name += MangleForJni(method_name);
    return short_name;
}

static std::string jni_long_name(const char *class_name,
                                 const char *method_name,
                                 const char *method_sig)
{
    std::string long_name;
    long_name += jni_short_name(class_name, method_name);
    long_name += "__";

    std::string signature(method_sig + 1);
    signature.erase(signature.find_last_of(')'));

    long_name += MangleForJni(signature);
    return long_name;
}


/**
* Android's code end
*/