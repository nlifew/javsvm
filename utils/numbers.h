


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
     * 这个函数用来对数字进行散列，以期望得到比较平均的值
     * 设置这个函数是因为 std::hash 在计算纯数字时，会直接返回，不会做进一步的处理.
     * 大多数情况下这个设计不会有问题，但对于指针做 key 的 concurrent_map，情况就不一样了.
     * 指针相比于纯数字有个特性，它会对齐到某个内存边界，这就导致指针的低几位经常都是 0,
     * 这样在访问 segment 时，就会总是访问到同一个 segment，导致多线程性能下降。
     *
     * 比如 0x1234567890，0x1234567880, 0x1234567870, 0x1234567860，
     * 在 segment 数少于 16 时，(ptr & 15) 总是等于 0
     *
     * 下面这个仅仅是非常简陋的实现，如果有更好的方案，可以随时修改之。
     *
     * @param val 输入的值
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

