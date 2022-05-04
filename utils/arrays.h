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

    template<typename T, typename Cmp = std::less<T>>
    static int bsearch(const T &key, const T *base, int n) noexcept
    {
        int lo = 0, hi = n - 1;
        while (lo <= hi) {
            int mid = (lo + hi) >> 1;
            const auto &mid_val = base[mid];
            if (Cmp()(mid_val, key)) {
                lo = mid + 1;
            }
            else if (Cmp(key, mid_val)) {
                hi = mid - 1;
            }
            else {
                return mid;
            }
        }
        return -(lo + 1);
    }

    static int bsearch(const void *key, const void *base, int n, size_t ele_size, int (*cmp)(const void *, const void *)) noexcept
    {
        int lo = 0, hi = n - 1;
        while (lo <= hi) {
            int mid = (lo + hi) >> 1;
            const void *mid_val = ((char *) base) + ele_size * mid;
            int result = cmp(key, mid_val);
            if (result < 0) {
                hi = mid - 1;
            }
            else if (result > 0) {
                lo = mid + 1;
            }
            else {
                return mid;
            }
        }
        return -(lo + 1);
    }

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
