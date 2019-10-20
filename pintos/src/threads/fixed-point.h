#ifndef THREADS_FIXED_POINT_H
#define THREADS_FIXED_POINT_H

#include <stdint.h>

/*32 bit fixed point representation with 17 bit for real part and 14 bit for fractional part and 1 bit as signed bit */
typedef int32_t fp_t;

fp_t convert_to_fp(int n);

/* Convert fp to integer rounding to zero */
int convert_fp_to_integer(fp_t fp);

/* Convert fp to integer rounding to nearest integer */
int convert_fp_to_integer_nearest(fp_t fp);

fp_t add_fp(fp_t fp, fp_t fp1);

fp_t sub_fp(fp_t fp, fp_t fp1);

fp_t add_int_to_fp(fp_t fp, int n);

fp_t sub_int_from_fp(fp_t fp, int n);

fp_t mul_fp(fp_t fp, fp_t fp1);

fp_t mul_fp_by_int(fp_t fp, int n);

fp_t div_fp(fp_t fp, fp_t fp1);

fp_t div_fp_by_int(fp_t fp, int n);

#endif /* threads/fixed_point.h */
