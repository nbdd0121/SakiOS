/**
 * Implement of math.h in ANSI C.
 *
 * @author Gary Guo <nbdd0121@hotmail.com>
 */

#ifndef C_MATH_H
#define C_MATH_H

int isnan(double x);
int isinf(double x);
double fabs(double x);
double log2(double x);
double log10(double x);
double log(double x);
double exp2(double x);
double pow(double x, double y);
double floor(double x);
double fmod(double x, double y);

#endif