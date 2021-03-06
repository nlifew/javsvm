


#ifndef _UTILS_NUMBER_UTILS
#define _UTILS_NUMBER_UTILS


#include <cstdint>
#include <type_traits>

namespace javsvm
{

struct numbers
{
    struct Integer {
        static constexpr int MIN = 1 << 31; // 0x80000000
        static constexpr int MAX = ~MIN;    // 0x7FFFFFFF
    };

    struct Float {
    public:
        static /*constexpr*/ float POSITIVE_INFINITY; // = 1.0f / 0.0f;     // *(float*) &(0x7f800000)
        static /*constexpr*/ float NEGATIVE_INFINITY; // = -1.0f / 0.0f;    // *(float*) &(0xff800000)
        static /*constexpr*/ float NaN; //= 0.0f / 0.0f;                   // *(float*) &(0x7fc00000)
    };

    struct Double {
    public:
        static /*constexpr*/ double POSITIVE_INFINITY; // = 1.0 / 0.0;      // 0x7ff00000_00000000L
        static /*constexpr*/ double NEGATIVE_INFINITY; // = -1.0 / 0.0;     // 0xfff00000_00000000L
        static /*constexpr*/ double NaN; // = ((double) 0.0) / 0.0;         // 0x7ff80000_00000000L
    };

    template <typename T> static 
    typename std::enable_if<sizeof(T) == sizeof(int8_t), T>::type
    reverse_endian(T val)
    {
        return val;
    }

    template <typename T> static 
    typename std::enable_if<sizeof(T) == sizeof(int16_t), T>::type
    reverse_endian(T val)
    {
        auto buff = (unsigned char *)&val;
        T tmp = buff[0] << 8;
        tmp |= buff[1];
        return tmp;
    }

    template <typename T> static 
    typename std::enable_if<sizeof(T) == sizeof(int32_t), T>::type
    reverse_endian(T val)
    {
        auto buff = (unsigned char *)&val;
        T tmp = buff[0] << 24;
        tmp |= buff[1] << 16;
        tmp |= buff[2] << 8;
        tmp |= buff[3];
        return tmp;
    }

    template <typename T> static 
    typename std::enable_if<sizeof(T) == sizeof(int64_t), T>::type
    reverse_endian(T val)
    {
        auto buff = (unsigned char *)&val;
        T tmp = (uint64_t) buff[0] << 56;
        tmp |= (uint64_t) buff[1] << 48;
        tmp |= (uint64_t) buff[2] << 40;
        tmp |= (uint64_t) buff[3] << 32;
        tmp |= buff[4] << 24;
        tmp |= buff[5] << 16;
        tmp |= buff[6] << 8;
        tmp |= buff[7];
        return tmp;
    }

    /**
     * ???????????????????????????????????????????????????????????????????????????
     * ??????????????????????????? std::hash ?????????????????????????????????????????????????????????????????????.
     * ?????????????????????????????????????????????????????????????????? key ??? concurrent_map????????????????????????.
     * ????????????????????????????????????????????????????????????????????????????????????????????????????????????????????? 0,
     * ??????????????? segment ???????????????????????????????????? segment?????????????????????????????????
     *
     * ?????? 0x1234567890???0x1234567880, 0x1234567870, 0x1234567860???
     * ??? segment ????????? 16 ??????(ptr & 15) ???????????? 0
     *
     * ????????????????????????????????????????????????????????????????????????????????????????????????
     *
     * @param val ????????????
     * @return hash
     */
    inline static int hash(size_t val) noexcept
    {
        size_t hash = val;
        hash ^= (val >> 1);
        hash ^= (val >> 2);
        hash ^= (val >> 4);
        hash ^= (val >> 8);
        hash ^= (val >> 16);
        hash ^= (val >> 32);
        return (int) (hash ^ (hash >> 16));
    }

    // template <typename T>
    // static T reverse_endian(T t)
    // {
    //     T cpy = t;

    //     char *l = (char*) &cpy;
    //     char *r = l + sizeof(T) - 1;
    //     while (l < r) {
    //         char swap = *l;
    //         *l = *r;
    //         *r = swap;

    //         l ++;
    //         r --;
    //     }
    //     return cpy;
    // }
};
};
#endif

