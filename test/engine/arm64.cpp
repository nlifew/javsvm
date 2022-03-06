//
// Created by edz on 2021/12/6.
//

#include <cstdint>
#include <cstdio>

extern "C"
int64_t calljni64(const void *addr, int ret_type, const int64_t *x,
               double *f, int stack_len, const void *stack);

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

static double f2d(float f) noexcept
{
    double d;
    *(float *) &d = f;
    return d;
}


int main(int argc, const char *argv[])
{

    int64_t x[8] = {
            1, 2, 3, 4, 5, 6, 7, 8,
    };
    double d[16] = {
            f2d(1.0F), 0.9, f2d(1.1F), 0.8,
            f2d(1.2F), 0.7, 0, 0,
            0, 0, 0, 0,
            0, 0, 0, 0
    };

    struct {
        char ch = 9;
        short st = 10;
        int in = 11;
        int64_t l = 12;
    } arg;

     int64_t ret_buff = calljni64(
             (void*) &func, 3,
              x, d, sizeof(arg), &arg);

    double a = *(double *) &ret_buff;
    double b = func(1, 2, 3, 4, 1.0F, 0.9,
                    5, 6, 7, 8, 1.1F, 0.8,
                    9, 10, 11, 12, 1.2F, 0.7);

    printf("%f, %f, %d\n", a, b, a == b);
}