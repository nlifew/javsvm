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


    template <typename T>
    static T* binsert(const T &key, T *base, int n) noexcept
    {
        int lo = 0, hi = n - 1;

        while (lo <= hi) {
            int mid = (lo + hi) >> 1;
            T &mid_val = base[mid];

            if (mid_val < key) {
                lo = mid + 1;
            }
            else if (mid_val > key) {
                hi = mid - 1;
            }
            else {
                return &mid_val;
            }
        }


        T *ret = base + lo;
        memmove(ret + 1, ret, (n - lo) * sizeof(T));
        *ret = key;

        return ret;
    }


};
}

#endif //JAVSM_ARRAY_UTILS_H
