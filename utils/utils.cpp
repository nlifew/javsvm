//
// Created by edz on 2021/11/22.
//

#include "numbers.h"

using namespace javsvm;

static constexpr uint32_t FLOAT_POSITIVE_INFINITY = 0x7f800000;
static constexpr uint32_t FLOAT_NEGATIVE_INFINITY = 0xff800000;
static constexpr uint32_t FLOAT_NaN = 0x7fc00000;



float numbers::Float::POSITIVE_INFINITY = *(float*) FLOAT_POSITIVE_INFINITY;
float numbers::Float::NEGATIVE_INFINITY = *(float*) FLOAT_NEGATIVE_INFINITY;
float numbers::Float::NaN = *(float*) FLOAT_NaN;


static constexpr uint64_t DOUBLE_POSITIVE_INFINITY = 0x7ff0000000000000L;
static constexpr uint64_t DOUBLE_NEGATIVE_INFINITY = 0xfff0000000000000L;
static constexpr uint64_t DOUBLE_NaN = 0x7ff8000000000000L;

double numbers::Double::POSITIVE_INFINITY = *(double*) DOUBLE_POSITIVE_INFINITY;
double numbers::Double::NEGATIVE_INFINITY = *(double*) DOUBLE_NEGATIVE_INFINITY;
double numbers::Double::NaN = *(double*) DOUBLE_NaN;