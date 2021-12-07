//
// Created by edz on 2021/12/6.
//

#include <cstdint>
#include <cstdio>

extern "C"
void calljni64(const void *addr, int ret_type, void *ret,
               const int64_t *x, int stack_len, const void *stack,
               int fargc, const int *fargctl, const uint64_t *fargv);

extern "C"
double func(char ch, short st, int in, int64_t l, float f, double d,
            char ch1, short st1, int in1, int64_t l1, float f1, double d1,
            char ch2, short st2, int in2, int64_t l2, float f2, double d2)
{
#if 0
    return 0;
#else
    int64_t hash = ch ^ st ^ in ^ l ^ ch1 ^ st1 ^ in1 ^ l1;
    hash ^= ch2 ^ st2 ^ in2 ^ l2;
    double dou = hash * f * f1 * f2;
    dou /= d / d1 / d2;
    return dou;
#endif
}



int main(int argc, const char *argv[])
{

    int64_t x[] = {
            1, 2, 3, 4, 5, 6, 7, 8,
    };

    struct {
        char ch = 9;
        short st = 10;
        int in = 11;
        int64_t l = 12;
    } arg;

    int fargctl[] = {
            0, 1, 0, 1, 0, 1
    };
    uint64_t fargv[6];
    *(float *) (fargv + 0) = 1.0F;
    *(double *) (fargv + 1) = 0.9;
    *(float *) (fargv + 2) = 1.1F;
    *(double *) (fargv + 3) = 0.8;
    *(float *) (fargv + 4) = 1.2F;
    *(double *) (fargv + 5) = 0.7;

    double ret = 0;
    calljni64((void*) &func, 3, &ret,
              x, sizeof(arg), &arg,
              sizeof(fargv) / sizeof(fargv[0]), fargctl, fargv);

    double a = ret;
    double b = func(1, 2, 3, 4, 1.0F, 0.9,
                    5, 6, 7, 8, 1.1F, 0.8,
                    9, 10, 11, 12, 1.2F, 0.7);

    printf("%d\n", a == b);
}