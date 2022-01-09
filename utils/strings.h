

#ifndef JAVSVM_STRINGS_H
#define JAVSVM_STRINGS_H

#include <cstring>
#include <cstdlib>

namespace javsvm
{

struct strings
{
    static std::wstring towstring(const char *src)
    {
        if (src == nullptr) {
            return {};
        }

        size_t bytes_len = strlen(src);
        auto *buff = new wchar_t[bytes_len];

        size_t char_len = mbstowcs(buff, src, bytes_len);
        std::wstring wstr;

        if (char_len != -1) {
            wstr.append(buff, char_len);
        }
        delete[] buff;
        return wstr;
    }

    static std::string tostring(const wchar_t *src)
    {
        if (src == nullptr) {
            return {};
        }
        size_t chars_len = wcslen(src);
        auto *buff = new char[chars_len * 4]; // 按照宽字符串的 4 倍申请空间

        size_t bytes_len = wcstombs(buff, src, chars_len * 3);
        std::string str;

        if (bytes_len != -1) {
            str.append(buff);
        }
        delete[] buff;
        return str;
    }


};

} // namespace javsvm

#endif
