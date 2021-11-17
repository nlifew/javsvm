

#ifndef JAVSVM_STRING_UTILS_H
#define JAVSVM_STRING_UTILS_H

#include <cstring>
#include <cstdlib>

namespace javsvm
{

struct string_utils
{

    static bool is_empty(const char *str) { return str == nullptr || *str == '\0'; }

    static bool strequ(const char *s1, const char *s2)
    {
        return s1 == s2 || s1 && s2 && ::strcmp(s1, s2) == 0;
    }

    static wchar_t* towstring(const char *src)
    {
        if (src == nullptr) {
            return nullptr;
        }

        size_t bytes_len = strlen(src);
        wchar_t *buff = new wchar_t[bytes_len];

        size_t char_len = mbstowcs(buff, src, bytes_len);
        if (char_len == -1) {
            delete[] buff;
            return nullptr;
        }
        return buff;
    }

    static char *tostring(const wchar_t *src);
};

} // namespace javsvm

#endif
