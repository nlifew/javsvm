

#ifndef JAVSVM_STRINGS_H
#define JAVSVM_STRINGS_H

#include <cstring>
#include <cstdlib>

namespace javsvm
{

struct strings
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
        auto *buff = new wchar_t[bytes_len];

        size_t char_len = mbstowcs(buff, src, bytes_len);
        if (char_len == -1) {
            delete[] buff;
            return nullptr;
        }
        return buff;
    }

    static char *tostring(const wchar_t *src)
    {
        if (src == nullptr) {
            return nullptr;
        }
        size_t chars_len = wcslen(src);
        auto *buff = new char[chars_len * 3]; // 按照宽字符串的 3 倍申请空间

        size_t bytes_len = wcstombs(buff, src, chars_len * 3);
        if (bytes_len == -1) {
            delete[] buff;
            return nullptr;
        }
        return buff;
    }
};

} // namespace javsvm

#endif
