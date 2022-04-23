

#include <cstring>
#include "utils/strings.h"

using namespace javsvm;

#define STRING {\
'H', 'e', 'l', 'l', 'o', ',', ' ', 'w', 'o', 'r', 'l', 'd', ' ', '!', '\0' \
}

int main()
{
    const char str[] = STRING;
    const jchar wstr[] = STRING;

    {
        size_t len = 0;
        auto wstr_cpy = strings::to_wstring(str, &len);
        assert(len == strlen(str));
        assert(memcmp(wstr_cpy, wstr, sizeof(wstr)) == 0);
        delete wstr_cpy;
    }
    {
        auto str_cpy = strings::to_string(wstr, sizeof(wstr) / sizeof(wstr[0]) - 1);
        assert(strcmp(str_cpy, str) == 0);
        delete[] str_cpy;
    }

    return 0;
}