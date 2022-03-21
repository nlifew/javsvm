//
// Created by nlifew on 2020/10/15.
//

#ifndef JAVSVM_GLOBAL_H
#define JAVSVM_GLOBAL_H


#if _WIN32 || WIN32 || WIN64
#define WINDOWS 1
#else
#define WINDOWS 0
#endif



#include <cstdint>
#include <cstdlib>
#include <cstring>

namespace javsvm
{

using   u1  =   uint8_t;
using   u2  =   uint16_t;
using   u4  =   uint32_t;
using   u8  =   uint64_t;

using   jboolean    =   uint8_t;    /* unsigned 8 bits */
using   jbyte       =   int8_t;     /* signed 8 bits */
using   jchar       =   uint16_t;   /* unsigned 16 bits */
using   jshort      =   int16_t;    /* signed 16 bits */
using   jint        =   int32_t;    /* signed 32 bits */
using   jlong       =   int64_t;    /* signed 64 bits */
using   jfloat      =   float;      /* 32-bit IEEE 754 */
using   jdouble     =   double;     /* 64-bit IEEE 754 */
using   jsize       =   jint;


struct _jref_struct
{
    void *do_not_use_me = nullptr;
};

using jref = _jref_struct*;


typedef union
{
    jboolean    z;
    jbyte       b;
    jchar       c;
    jshort      s;
    jint        i;
    jlong       j;
    jfloat      f;
    jdouble     d;
    jref        l;
} jvalue;

template <int size, typename T>
static inline T align(T in) noexcept
{
    static_assert((size & (size - 1)) == 0);
    return ((in - 1) | (size - 1)) + 1;
}


template <typename T>
static int _slotof() { return align<4>(sizeof(T)) >> 2; }

template <>
int _slotof<jref>() { return 1; }

template <>
int _slotof<void>() { return 0; }

#define slotof(T) _slotof<T>()

#define INLINE __attribute__((always_inline))

#define LIKELY(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)

#define if_likely(x) if (LIKELY(x))
#define if_unlikely(x) if (UNLIKELY(x))
}

#endif //JAVSVM_GLOBAL_H
