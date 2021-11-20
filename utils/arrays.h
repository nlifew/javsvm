//
// Created by edz on 2021/11/14.
//

#ifndef JAVSM_ARRAY_UTILS_H
#define JAVSM_ARRAY_UTILS_H

#include <cstdlib>

namespace javsvm
{
struct arrays
{
    template<typename T>
    static int binary_search(const T& key, const T* from, int size, int (*cmp)(const T*, const T*))
    {
        using cmp_t = int (*)(const void*, const void*);
        T* old = (T*) std::bsearch(&key, from, size, sizeof(T), (cmp_t )cmp);
        if (old == nullptr) return -1;

        return (int) (old - from);
    }
};
}

#endif //JAVSM_ARRAY_UTILS_H
