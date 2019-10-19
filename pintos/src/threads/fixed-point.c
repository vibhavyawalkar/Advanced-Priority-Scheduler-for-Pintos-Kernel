#include "threads/fixed-point.h"

fp_t convert_to_fp(int n)
{
  //Multiplying n by 2^14
  fp_t fp = n << 14;
  return fp;
}

/* Convert fp to integer rounding to zero */
int convert_fp_to_integer(fp_t fp)
{
  //Dividing fp by 2^14
  int n = fp >> 14;
  return n;
}

/* Convert fp to integer rounding to nearest integer */
int convert_fp_to_integer_nearest(fp_t fp)
{
  int n;
  if (fp >= 0) {
    n = (fp + (1 << 13))/(1 << 14);
  } else {
    n = (fp - (1 << 13))/(1 << 14);
  }
  return n;
}

fp_t add_fp(fp_t fp, fp_t fp1)
{
  return (fp+fp1);
}

fp_t sub_fp(fp_t fp, fp_t fp1)
{
  return (fp-fp1);
}

fp_t add_int_to_fp(fp_t fp, int n)
{
  fp_t fp_ = fp + (n << 14);
  return fp_;
}

fp_t sub_int_from_fp(fp_t fp, int n)
{
  fp_t fp_ = fp - (n << 14);
  return fp_;
}

fp_t mul_fp(fp_t fp, fp_t fp1)
{
  fp_t fp_ = (((int64_t)fp)*(fp1 >>14));
  return fp_;
}

fp_t mul_fp_by_int(fp_t fp, int n)
{
  fp_t fp_ = fp * n;
  return fp_;
}

fp_t div_fp(fp_t fp, fp_t fp1)
{
  fp_t fp_ = (((int64_t)fp)/(fp1 >> 14));
  return fp_;
}

fp_t div_fp_by_int(fp_t fp, int n)
{
  fp_t fp_ = fp/n;
  return fp_;
}
