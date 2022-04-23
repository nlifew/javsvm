

#ifndef JAVSVM_STRINGS_H
#define JAVSVM_STRINGS_H


#include <string>

#include "global.h"

namespace javsvm
{

struct strings
{
    static const jchar* to_wstring(const char *src, size_t *wstr_len) noexcept;

    static const char *to_string(const jchar *src, size_t len) noexcept;

    static size_t to_string(char *dst, const jchar *src, size_t len) noexcept;
};

} // namespace javsvm

#endif
