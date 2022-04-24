//
// Created by edz on 2021/11/22.
//

#include "numbers.h"

using namespace javsvm;

static constexpr uint32_t FLOAT_POSITIVE_INFINITY = 0x7f800000;
static constexpr uint32_t FLOAT_NEGATIVE_INFINITY = 0xff800000;
static constexpr uint32_t FLOAT_NaN = 0x7fc00000;



float numbers::Float::POSITIVE_INFINITY = *(float*) &FLOAT_POSITIVE_INFINITY;
float numbers::Float::NEGATIVE_INFINITY = *(float*) &FLOAT_NEGATIVE_INFINITY;
float numbers::Float::NaN = *(float*) &FLOAT_NaN;


static constexpr uint64_t DOUBLE_POSITIVE_INFINITY = 0x7ff0000000000000L;
static constexpr uint64_t DOUBLE_NEGATIVE_INFINITY = 0xfff0000000000000L;
static constexpr uint64_t DOUBLE_NaN = 0x7ff8000000000000L;

double numbers::Double::POSITIVE_INFINITY = *(double*) &DOUBLE_POSITIVE_INFINITY;
double numbers::Double::NEGATIVE_INFINITY = *(double*) &DOUBLE_NEGATIVE_INFINITY;
double numbers::Double::NaN = *(double*) &DOUBLE_NaN;


#include "strings.h"
#include "log.h"
#include <iconv.h>
#include <cerrno>

struct
{
    iconv_t utf8_utf16 = nullptr;
    iconv_t utf16_utf8 = nullptr;

    void ensure_created() noexcept
    {
        if (utf8_utf16 == nullptr && (utf8_utf16 = iconv_open("UTF-16LE", "UTF-8")) == (iconv_t) -1) {
            PLOGE("towstring: failed to alloc utf8->utf16 iconv\n");
            exit(1);
        }
        if (utf16_utf8 == nullptr && (utf16_utf8 = iconv_open("UTF-8", "UTF-16LE")) == (iconv_t) -1) {
            PLOGE("tostring: failed to alloc utf16->utf8 iconv\n");
            exit(1);
        }
    }

} handle;

const jchar *strings::to_wstring(const char *src, size_t *wstr_len) noexcept
{
    if (wstr_len != nullptr) {
        *wstr_len = 0;
    }
    handle.ensure_created();

    char *in_buf = const_cast<char*>(src);
    size_t in_len = strlen(in_buf) + 1;

    auto buff = new jchar[in_len];
    size_t buff_len = in_len * sizeof(jchar);

    char *out_buf = reinterpret_cast<char*>(buff);
    size_t out_len = buff_len;

    iconv(handle.utf8_utf16, &in_buf, &in_len, &out_buf, &out_len);

    if (wstr_len != nullptr) {
        *wstr_len = (buff_len - out_len) / sizeof(jchar) - 1; // [1]
        // [1]. 先除以 sizeof(jchar) 再减去 1，相当于删去了 2 个字节，
        // 即 UTF-16 末尾的 0x00, 0x00
    }
    return buff;
}

/**
 * utf16 转 utf8 时的扩容倍数。对于 ascii，这个倍率维持在 1.2 完全够用，
 * 但对中文，这个值要增加在 3.0 才够用
 */
static constexpr int MULTI = 3;


const char *strings::to_string(const jchar *src, size_t len) noexcept
{
    handle.ensure_created();

    size_t buff_len = 1 + (MULTI * len);
    char *buff = new char[buff_len];

    char *in_buf = (char *) src;
    size_t in_len = (len + 1) * sizeof(jchar);

    char *out_buf = buff;
    size_t out_len = buff_len;

    iconv(handle.utf16_utf8, &in_buf, &in_len, &out_buf, &out_len);

    buff[buff_len - out_len] = '\0';
    return buff;
}

size_t strings::to_string(char *dst, const jchar *src, size_t len) noexcept
{
    dst[0] = '\0';
    handle.ensure_created();

    char *in_buf = (char *) src;
    size_t in_len = (len + 1) * sizeof(jchar);

    size_t buf_len = 5 * in_len; // 认为输出缓冲区已经足够大

    char *out_buf = dst;
    size_t out_len = in_len * 5;

    iconv(handle.utf16_utf8, &in_buf, &in_len, &out_buf, &out_len);

    dst[buf_len - out_len] = '\0';
    return buf_len - out_len;
}