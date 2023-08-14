#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#define SIGN 31
#define START_SCALE 16
#define MAX_INT 2147483647
#define MAX_UINT 4294967295
#define DEC_ARRAY_PART_MAX_SIZE 29
#define DECIMAL_MANTISSA_SIZE 96
#define OK 0
#define CONVERSION_ERROR 1
#define POSITIVE_INFINITY 1
#define NEGATIVE_INFINITY 2
#define DIVISION_BY_ZERO 3

typedef struct {
  int bits[4];
} s21_decimal;

/// @brief Структура - представление числа в виде массива десятичных цифр целой
/// и дробной части
/// @param sign Знак числа (0 - '+', 1 - '-')
/// @param integerPartSize Количество цифр в целой части числа (0 - 29)
/// @param integerPart Массив цифр целой части числа ([0] - разряд единиц)
/// @param floatPartSize Количество цифр в дробной части числа (0 - 29)
/// @param floatPart Массив цифр дробной части числа ([0] - разряд десятых
/// долей)
typedef struct {
  int sign;
  int integerPartSize;
  int integerPart[29];
  int floatPartSize;
  int floatPart[29];
} decArray;

int s21_add(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);
int s21_sub(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);
int s21_mul(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);
int s21_div(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);

int s21_is_equal(s21_decimal value_1, s21_decimal value_2);
int s21_is_not_equal(s21_decimal value_1, s21_decimal value_2);
int s21_is_less(s21_decimal value_1, s21_decimal value_2);
int s21_is_less_or_equal(s21_decimal value_1, s21_decimal value_2);
int s21_is_greater(s21_decimal value_1, s21_decimal value_2);
int s21_is_greater_or_equal(s21_decimal value_1, s21_decimal value_2);

int s21_from_int_to_decimal(int src, s21_decimal *dst);
int s21_from_float_to_decimal(float src, s21_decimal *dst);
int s21_from_decimal_to_int(s21_decimal src, int *dst);
int s21_from_decimal_to_float(s21_decimal src, float *dst);

int s21_floor(s21_decimal value, s21_decimal *result);
int s21_round(s21_decimal value, s21_decimal *result);
int s21_truncate(s21_decimal value, s21_decimal *result);
int s21_negate(s21_decimal value, s21_decimal *result);

int getSign(s21_decimal src);
int getExponent(s21_decimal src);
void writeExponent(s21_decimal *src, int exponent);
int cmp_no_scale_no_sign(s21_decimal value_1, s21_decimal value_2);
void decimal_copy(s21_decimal *tmp_1, s21_decimal value_1);
void do_equal_scale(s21_decimal *value_1, s21_decimal *value_2);
int getBit(s21_decimal src, int number);
void writeBit(s21_decimal *src, int number, int bit);
void writeSign(s21_decimal *src, int sign);
decArray getDecimalAsDecArray(s21_decimal src);
void getPowerOf2(int *arr, int exp);
void sumArr(int *arr1, int *arr2);
int countIntegerPartSize(decArray arr);
int countFloatPartSize(decArray arr);
void multiplyBy(int *mantissa, int factor);
decArray decArrayAdd(decArray src1, decArray src2, int *exception);
decArray decArraySub(decArray src1, decArray src2, int *exception);
s21_decimal getDecArrayAsDecimal(decArray src);
int compareAbsDecArray(decArray src1, decArray src2);
int makeInteger(decArray *src);
decArray decArrayMul(decArray src1, decArray src2, int *exception);
decArray decArrayDiv(decArray dividend, decArray divisor, int *exception);
decArray decArrayAbsAdd(decArray src1, decArray src2);
decArray decArrayAbsSub(decArray src1, decArray src2);
int isZero(decArray src);
void multiplyByPow10(int exp, decArray *src);
void divideByPow10(int exp, decArray *src);

int rounding(int lastBit, int bitAfterLast);
decArray bankRoundDecArray(decArray src);
decArray maxDecArray();
