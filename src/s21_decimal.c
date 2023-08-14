#include "s21_decimal.h"

// -------------функции сравнения---------------------

/// @brief Функция проверяет два s21_decimal на равенство
/// @param value_1 - первое число s21_decimal
/// @param value_2 - второе число s21_decimal
/// @return 0 - (ложь)числа не равны, 1 - (истина)числа равны
int s21_is_equal(s21_decimal value_1, s21_decimal value_2) {
  int result = 1;
  s21_decimal tmp_1 = {0};
  s21_decimal tmp_2 = {0};
  decimal_copy(&tmp_1, value_1);
  decimal_copy(&tmp_2, value_2);
  if (getSign(tmp_1) != getSign(tmp_2)) {
    result = 0;
  }
  if (result == 1) {
    if (getExponent(tmp_1) != getExponent(tmp_2)) {
      do_equal_scale(&tmp_1, &tmp_2);
    }
  }
  if (result == 1) {
    for (int i = 0; i < 3 && result == 1; i++) {
      if (tmp_1.bits[i] != tmp_2.bits[i]) {
        result = 0;
      }
    }
  }
  return result;
}

/// @brief Функция проверяет два s21_decimal на НЕравенство
/// @param value_1 - первое число s21_decimal
/// @param value_2 - второе число s21_decimal
/// @return 0 - (ложь)числа равны, 1 - (истина)числа не равны
int s21_is_not_equal(s21_decimal value_1, s21_decimal value_2) {
  return !s21_is_equal(value_1, value_2);
}

/// @brief Проверка меньше первое число, чем второе
/// @param value_1 первое число s21_decimal
/// @param value_2 второе число s21_decimal
/// @return 0 - ложь, 1 - истина
int s21_is_less(s21_decimal value_1, s21_decimal value_2) {
  int result = -1;
  int sign = 1;
  if ((sign = getSign(value_1)) != getSign(value_2)) {
    if (sign == 0)
      result = 0;
    else
      result = 1;
  }
  if (result == -1) {
    s21_decimal tmp_1 = {0};
    s21_decimal tmp_2 = {0};
    decimal_copy(&tmp_1, value_1);
    decimal_copy(&tmp_2, value_2);
    int scale_1 = getExponent(value_1);
    int scale_2 = getExponent(value_2);
    if (scale_1 != scale_2) {
      do_equal_scale(&tmp_1, &tmp_2);
    }
    int result_cmp = cmp_no_scale_no_sign(tmp_1, tmp_2);
    if (result_cmp == -1)
      result = 1;
    else
      result = 0;
    if (sign && result_cmp != 0) result = !result;
  }
  return result;
}

/// @brief Проверка меньше или равно первое число, чем второе
/// @param value_1 первое число s21_decimal
/// @param value_2 второе число s21_decimal
/// @return 0 - ложь, 1 - истина
int s21_is_less_or_equal(s21_decimal value_1, s21_decimal value_2) {
  return s21_is_less(value_1, value_2) || s21_is_equal(value_1, value_2);
}

/// @brief Проверка больше первое число, чем второе
/// @param value_1 первое число s21_decimal
/// @param value_2 второе число s21_decimal
/// @return 0 - ложь, 1 - истина
int s21_is_greater(s21_decimal value_1, s21_decimal value_2) {
  return !s21_is_less(value_1, value_2) && !s21_is_equal(value_1, value_2);
}

/// @brief Проверка больше или равно первое число, чем второе
/// @param value_1 первое число s21_decimal
/// @param value_2 второе число s21_decimal
/// @return 0 - ложь, 1 - истина
int s21_is_greater_or_equal(s21_decimal value_1, s21_decimal value_2) {
  return s21_is_greater(value_1, value_2) || s21_is_equal(value_1, value_2);
}

// -------------функции преобразования---------------------

/// @brief Преобразование int в s21_decimal
/// @param src int
/// @param dst указатель на число s21_decimal
/// @return 0 - Ок, 1 - ошибка конвертации
int s21_from_int_to_decimal(int src, s21_decimal *dst) {
  dst->bits[3] = (src >= 0 ? 0 : 1) << 31;
  if (src >= 0)
    dst->bits[0] = src;
  else {
    dst->bits[0] = -src;
  }
  dst->bits[1] = 0;
  dst->bits[2] = 0;
  return 0;
}

int fromCharToInt(char c) { return c - '0'; }

/// @brief Преобразование float в s21_decimal
/// @param src float
/// @param dst указатель на число s21_decimal
/// @return 0 - Ок, 1 - ошибка конвертации
int s21_from_float_to_decimal(float src, s21_decimal *dst) {
  if (isnan(src) || !isfinite(src)) return CONVERSION_ERROR;

  char string[256] = "";
  sprintf(string, "%+e", src);

  int mantissa = fromCharToInt(string[1]);
  for (int i = 0; i < 6; i++) {
    mantissa *= 10;
    mantissa += fromCharToInt(string[3 + i]);
  }

  int exponent = 0;
  for (int i = 11; string[i] != '\0'; i++) {
    exponent *= 10;
    exponent += fromCharToInt(string[i]);
  }
  if (string[10] == '-') exponent *= -1;
  exponent -= 6;

  while (exponent < -28 && mantissa != 0) {
    mantissa /= 10;
    exponent++;
  }

  if (exponent > 28) {
    return CONVERSION_ERROR;
  }
  if (exponent < -28) {
    s21_from_int_to_decimal(0, dst);
    return CONVERSION_ERROR;
  }

  if (exponent >= 0) {
    mantissa *= (int)pow(10, exponent);
    s21_from_int_to_decimal(mantissa, dst);
  } else {
    s21_from_int_to_decimal(mantissa, dst);
    writeExponent(dst, mantissa != 0 ? -exponent : 0);
  }
  writeSign(dst, string[0] == '+' ? 0 : 1);
  return OK;
}

/// @brief Преобразование s21_decimal в int
/// @param src число s21_decimal
/// @param dst указатель на int
/// @return 0 - Ок, 1 - ошибка конвертации
int s21_from_decimal_to_int(s21_decimal src, int *dst) {
  decArray arr = getDecimalAsDecArray(src);
  long long dst2 = 0;
  int result = 0;
  for (int i = 0; i < arr.integerPartSize && !result; i++) {
    dst2 += arr.integerPart[i] * pow(10, i);
    if (dst2 > MAX_INT) {
      result = 1;
    }
  }
  if (!result) {
    *dst = dst2;
    if (getSign(src) == 1) *dst *= -1;
  }
  return result;
}

/// @brief Преобразование s21_decimal в float
/// @param src число s21_decimal
/// @param dst указатель на float
/// @return 0 - Ок, 1 - ошибка конвертации
int s21_from_decimal_to_float(s21_decimal src, float *dst) {
  decArray arr = getDecimalAsDecArray(src);
  int exception = OK;

  double result = 0;
  for (int i = 0; i < arr.integerPartSize; i++) {
    result += arr.integerPart[i] * pow(10, i);
  }
  for (int i = 0; i < arr.floatPartSize; i++) {
    result += arr.floatPart[i] * pow(10, -i - 1);
  }
  if (getSign(src) == 1) result *= -1;

  // if ((float)result != result)
  //   exception = CONVERSION_ERROR;

  if (exception == OK) *dst = (float)result;
  return exception;
}

// -------------арифметические функции---------------------

/// @brief Рассчитывает сумму двух s21_decimal
/// @param value_1 Первое слагаемое
/// @param value_2 Второе слагаемое
/// @param result Указатель на сумму
/// @return '0' - OK, '1' - число слишком велико или равно бесконечности,
/// '2' - число слишком мало или равно отрицательной бесконечности, '3' -
/// деление на 0
int s21_add(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  decArray src1 = getDecimalAsDecArray(value_1);
  decArray src2 = getDecimalAsDecArray(value_2);
  decArray res = {0};
  int exception = 0;

  if (src1.sign == src2.sign) {
    res = decArrayAdd(src1, src2, &exception);
    res.sign = src1.sign;
  } else {
    int cADA = compareAbsDecArray(src1, src2);
    if (cADA == 1 || cADA == 0) {
      res = decArraySub(src1, src2, &exception);
      res.sign = src1.sign;
    } else {
      res = decArraySub(src2, src1, &exception);
      res.sign = src2.sign;
    }
  }

  *result = getDecArrayAsDecimal(res);
  if (exception == OK) return exception;
  if (src1.sign == src2.sign)
    return src1.sign == 0 ? POSITIVE_INFINITY : NEGATIVE_INFINITY;
  if (s21_is_greater(value_1, value_2) == 1)
    return src1.sign == 0 ? POSITIVE_INFINITY : NEGATIVE_INFINITY;
  return src2.sign == 0 ? POSITIVE_INFINITY : NEGATIVE_INFINITY;
}

/// @brief Рассчитывает разность двух s21_decimal
/// @param value_1 Уменьшаемое
/// @param value_2 Вычитаемое
/// @param result Указатель на разность
/// @return '0' - OK, '1' - число слишком велико или равно бесконечности,
/// '2' - число слишком мало или равно отрицательной бесконечности, '3' -
/// деление на 0
int s21_sub(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  decArray src1 = getDecimalAsDecArray(value_1);
  decArray src2 = getDecimalAsDecArray(value_2);
  decArray res = {0};
  int exception = OK;

  if (src1.sign != src2.sign) {
    res = decArrayAdd(src1, src2, &exception);
    res.sign = src1.sign;
  } else {
    int cADA = compareAbsDecArray(src1, src2);
    if (cADA == 1 || cADA == 0) {
      res = decArraySub(src1, src2, &exception);
      res.sign = src1.sign;
    } else {
      res = decArraySub(src2, src1, &exception);
      res.sign = (src2.sign == 0 ? 1 : 0);
    }
  }

  if (exception == OK) *result = getDecArrayAsDecimal(res);
  if (exception == OK) return exception;
  if (src1.sign == 1) return NEGATIVE_INFINITY;
  return src2.sign == 1 ? POSITIVE_INFINITY : NEGATIVE_INFINITY;
}

/// @brief Рассчитывает произведение двух s21_decimal
/// @param value_1 Первый множитель
/// @param value_2 Второй множитель
/// @param result Указатель на произведение
/// @return '0' - OK, '1' - число слишком велико или равно бесконечности,
/// '2' - число слишком мало или равно отрицательной бесконечности, '3' -
/// деление на 0
int s21_mul(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  decArray src1 = getDecimalAsDecArray(value_1);
  decArray src2 = getDecimalAsDecArray(value_2);
  int exception = OK;

  decArray res = decArrayMul(src1, src2, &exception);

  if (exception == OK) *result = getDecArrayAsDecimal(res);
  if (exception != OK)
    return src1.sign == src2.sign ? POSITIVE_INFINITY : NEGATIVE_INFINITY;
  return exception;
}

/// @brief Рассчитывает частное двух s21_decimal
/// @param value_1 Делимое
/// @param value_2 Делитель
/// @param result Указатель на частное
/// @return '0' - OK, '1' - число слишком велико или равно бесконечности,
/// '2' - число слишком мало или равно отрицательной бесконечности, '3' -
/// деление на 0
int s21_div(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  decArray dividend = getDecimalAsDecArray(value_1);
  decArray divisor = getDecimalAsDecArray(value_2);

  int exception = 0;
  decArray quotient = decArrayDiv(dividend, divisor, &exception);

  if (exception == OK) *result = getDecArrayAsDecimal(quotient);
  return exception;
}

// -----------------------Другие функции---------------------

/// @brief Округляет указанное Decimal число до ближайшего целого числа в
/// сторону отрицательной бесконечности
/// @param value Указанное Decimal число
/// @param result Указатель на результат
/// @return 0 - Ок, 1 - Ошибка вычисления
int s21_floor(s21_decimal value, s21_decimal *result) {
  decArray dividend = getDecimalAsDecArray(value);
  for (int i = 0; i < DEC_ARRAY_PART_MAX_SIZE; i++) {
    dividend.floatPart[i] = 0;
  }
  if (dividend.sign && dividend.floatPartSize != 0) dividend.integerPart[0]++;
  dividend.floatPartSize = 0;
  *result = getDecArrayAsDecimal(dividend);
  return 0;
}

/// @brief Округляет Decimal до ближайшего целого числа
/// @param value Указанное Decimal число
/// @param result Указатель на результат
/// @return 0 - Ок, 1 - Ошибка вычисления
int s21_round(s21_decimal value, s21_decimal *result) {
  decArray dividend = getDecimalAsDecArray(value);
  int plus = 0;
  if (dividend.floatPart[0] >= 5) plus = 1;
  dividend.integerPart[0] += plus;
  for (int i = 0; i < DEC_ARRAY_PART_MAX_SIZE; i++) {
    dividend.floatPart[i] = 0;
  }
  dividend.floatPartSize = 0;
  *result = getDecArrayAsDecimal(dividend);
  return 0;
}

/// @brief Возвращает целые цифры указанного Decimal числа; любые дробные цифры
/// отбрасываются, включая конечные нули
/// @param value Указанное Decimal число
/// @param result Указатель на результат
/// @return 0 - Ок, 1 - Ошибка вычисления
int s21_truncate(s21_decimal value, s21_decimal *result) {
  decArray dividend = getDecimalAsDecArray(value);
  for (int i = 0; i < DEC_ARRAY_PART_MAX_SIZE; i++) {
    dividend.floatPart[i] = 0;
  }
  dividend.floatPartSize = 0;
  *result = getDecArrayAsDecimal(dividend);
  return 0;
}

/// @brief Возвращает результат умножения указанного Decimal на -1.
/// @param value Указанное Decimal число
/// @param result Указатель на результат
/// @return 0 - Ок, 1 - Ошибка вычисления
int s21_negate(s21_decimal value, s21_decimal *result) {
  decimal_copy(result, value);
  if (value.bits[0] != 0 || value.bits[1] != 0 || value.bits[2] != 0) {
    writeSign(result, (getSign(value) - 1) * -1);
  }
  return 0;
}

// -------------------UTILITARY FUNCTIONS-------------------

/// @brief Вычитает один decArray из другого по модулю
/// @param src1 Уменьшаемое (src1 > src2)
/// @param src2 Вычитатель
/// @return Разность
decArray decArraySub(decArray src1, decArray src2, int *exception) {
  decArray result = {0};
  for (int i = 0; i < DEC_ARRAY_PART_MAX_SIZE; i++) {
    result.integerPart[i] = src1.integerPart[i] - src2.integerPart[i];
  }
  for (int i = 0; i < DEC_ARRAY_PART_MAX_SIZE; i++)
    result.floatPart[i] = src1.floatPart[i] - src2.floatPart[i];

  bool flag = false;
  while (!flag) {
    flag = true;
    for (int i = DEC_ARRAY_PART_MAX_SIZE - 1; i > 0; i--)
      if (result.floatPart[i] < 0) {
        flag = false;
        result.floatPart[i - 1] -= 1;
        result.floatPart[i] += 10;
      }
    if (result.floatPart[0] < 0) {
      flag = false;
      result.integerPart[0] -= 1;
      result.floatPart[0] += 10;
    }
    for (int i = 0; i < DEC_ARRAY_PART_MAX_SIZE; i++) {
      if (result.integerPart[i] < 0) {
        flag = false;
        result.integerPart[i + 1] -= 1;
        result.integerPart[i] += 10;
      }
    }
  }
  result.integerPartSize = countIntegerPartSize(result);
  result.floatPartSize = countFloatPartSize(result);
  *exception = 0;
  return result;
}

/// @brief Побайтовый беззнаковый и безстепенной компаратор
/// @param value_1 - первое число s21_decimal
/// @param value_2 - второе число s21_decimal
/// @return -1 value_1 меньше, 0 числа равны, 1 value_1 больше
int cmp_no_scale_no_sign(s21_decimal value_1, s21_decimal value_2) {
  int result = 0;
  for (int i = 2; i >= 0 && result == 0; i--) {
    if (value_1.bits[i] - value_2.bits[i] < 0) {
      result = -1;
    } else if (value_1.bits[i] - value_2.bits[i] > 0) {
      result = 1;
    }
  }
  return result;
}

/// @brief Функция копирует s21_decimal value_2 в s21_decimal value_1
/// @param value_1 - указатель на первое число s21_decimal
/// @param value_2 - второе число s21_decimal
void decimal_copy(s21_decimal *value_1, s21_decimal value_2) {
  for (int i = 0; i < 4; i++) {
    value_1->bits[i] = value_2.bits[i];
  }
}

/// @brief Функция приводит два s21_decimal к общей глубине
/// @param value_1 - первое число s21_decimal
/// @param value_2 - второе число s21_decimal
void do_equal_scale(s21_decimal *value_1, s21_decimal *value_2) {
  if (getExponent(*value_1) > getExponent(*value_2)) {
    do_equal_scale(value_2, value_1);
  }
  while (getExponent(*value_1) != getExponent(*value_2)) {
    long long bits_0 = value_1->bits[0] * 10;
    int transfer_0 = 0, transfer_1 = 0;
    if (bits_0 > MAX_UINT) transfer_0 = bits_0 / MAX_UINT;
    long long bits_1 = value_1->bits[1] * 10 + transfer_0;
    if (bits_1 > MAX_UINT) transfer_1 = bits_1 / MAX_UINT;

    long bits_2 = value_1->bits[2] * 10 + transfer_1;
    if (bits_2 > MAX_UINT) {
      s21_decimal result = {0};
      s21_decimal ten = {{10, 0, 0, 0}};
      s21_div(*value_1, ten, &result);
      decimal_copy(value_1, result);
      writeExponent(value_1, getExponent(*value_1) + 2);
    } else {
      value_1->bits[0] = bits_0 % MAX_UINT;
      value_1->bits[1] = bits_1 % MAX_UINT;
      value_1->bits[2] = bits_2 % MAX_UINT;
      writeExponent(value_1, getExponent(*value_1) + 1);
    }
  }
}

/// @brief Преобразует s21_decimal в decArray
/// @param src
/// @return decArray
decArray getDecimalAsDecArray(s21_decimal src) {
  decArray result = {0};
  result.sign = getSign(src);
  int mantissa[DEC_ARRAY_PART_MAX_SIZE];
  for (int i = 0; i < DEC_ARRAY_PART_MAX_SIZE; i++) mantissa[i] = 0;
  for (int i = 0; i < DECIMAL_MANTISSA_SIZE; i++)
    if (getBit(src, i) == 1) {
      int temp[29];
      getPowerOf2(temp, i);
      sumArr(mantissa, temp);
    }

  int exp = getExponent(src);
  for (int i = exp; i < DEC_ARRAY_PART_MAX_SIZE; i++)
    result.integerPart[i - exp] = mantissa[i];
  for (int i = 0; i < exp && exp != 0; i++)
    result.floatPart[exp - i - 1] = mantissa[i];

  result.integerPartSize = countIntegerPartSize(result);
  result.floatPartSize = countFloatPartSize(result);

  return result;
}

/// @brief Рассчитывает степень двух в виде массива десятичных цифр
/// @param arr
/// @param exp Степень двойки
void getPowerOf2(int *arr, int exp) {
  for (int i = 0; i < DEC_ARRAY_PART_MAX_SIZE; i++) arr[i] = 0;
  arr[0] = 1;
  for (int i = 0; i < exp; i++) multiplyBy(arr, 2);
}

/// @brief Прибавляет к первому массиву десятичных цифр второй
/// @param arr1 Первое слагаемое
/// @param arr2 Второе слагаемое
void sumArr(int *arr1, int *arr2) {
  for (int i = 0; i < DEC_ARRAY_PART_MAX_SIZE; i++) {
    arr1[i] += arr2[i];
    if (i != 28) {
      arr1[i + 1] += arr1[i] / 10;
      arr1[i] %= 10;
    }
  }
}

/// @brief Определяет длину целой части decArray
/// @param arr
/// @return Количество цифр в целой части decArray
int countIntegerPartSize(decArray arr) {
  int result = DEC_ARRAY_PART_MAX_SIZE;
  for (int i = DEC_ARRAY_PART_MAX_SIZE - 1; i >= 0 && arr.integerPart[i] == 0;
       i--)
    result--;
  return result;
}

/// @brief Определяет длину дробной части decArray
/// @param arr
/// @return Количество цифр в дробной части decArray
int countFloatPartSize(decArray arr) {
  int result = DEC_ARRAY_PART_MAX_SIZE;
  for (int i = DEC_ARRAY_PART_MAX_SIZE - 1; i >= 0 && arr.floatPart[i] == 0;
       i--)
    result--;
  return result;
}

/// @brief Умножает число представленное в виде массива десятичных цифр на
/// число
/// @param mantissa Умножаемое
/// @param factor Множитель
void multiplyBy(int *mantissa, int factor) {
  for (int i = 0; i < DEC_ARRAY_PART_MAX_SIZE; i++) mantissa[i] *= factor;

  bool flag = false;
  while (!flag) {
    flag = true;
    for (int i = 0; i < DEC_ARRAY_PART_MAX_SIZE; i++)
      if (mantissa[i] > 9 && i != DEC_ARRAY_PART_MAX_SIZE - 1) {
        flag = false;
        mantissa[i + 1] += mantissa[i] / 10;
        mantissa[i] %= 10;
      }
  }
}

/// @brief Переводит decArray в s21_decimal
/// @param src decArray для перевода
/// @return Результирующий s21_decimal
s21_decimal getDecArrayAsDecimal(decArray src) {
  src = bankRoundDecArray(src);
  s21_decimal result = {0};
  if (isZero(src)) return result;

  int BitArray[DECIMAL_MANTISSA_SIZE];
  for (int i = 0; i < DECIMAL_MANTISSA_SIZE; i++) BitArray[i] = 0;
  int Bit = 0;

  int exp = makeInteger(&src);

  while (countIntegerPartSize(src) != 0 && Bit < DECIMAL_MANTISSA_SIZE) {
    int remainder = 0;
    for (int i = src.integerPartSize - 1; i >= 0; i--) {
      src.integerPart[i] += 10 * remainder;
      remainder = src.integerPart[i] % 2;
      src.integerPart[i] /= 2;
    }
    BitArray[Bit++] = remainder;
  }

  for (int i = 0; i < DECIMAL_MANTISSA_SIZE; i++)
    writeBit(&result, i, BitArray[i]);

  writeSign(&result, src.sign);
  writeExponent(&result, exp);

  return result;
}

/// @brief Получает бит мантиссы s21_decimal на заданной позиции
/// @param src
/// @param number Позиция бита [0-95]
/// @return Значение бита
int getBit(s21_decimal src, int number) {
  return (src.bits[number / 32] >> (number % 32)) & 1;
}

/// @brief Записывает в s21_decimal бит на соответствующую позицию
/// @param src Указатель на s21_decimal для записи
/// @param number Позиция для записи [0-95]
/// @param bit Бит
void writeBit(s21_decimal *src, int number, int bit) {
  int res = 0;
  for (int i = 0; i < 32; i++)
    res |= (i == (number % 32) ? bit : getBit(*src, (number / 32) * 32 + i))
           << i;
  src->bits[number / 32] = res;
}

/// @brief Записывает в s21_decimal бит знака
/// @param src
/// @param sign
void writeSign(s21_decimal *src, int sign) {
  src->bits[3] = (sign << SIGN) | (getExponent(*src) << START_SCALE);
}

/// @brief Записывает в s21_decimal значение экспоненты
/// @param src
/// @param exponent
void writeExponent(s21_decimal *src, int exponent) {
  src->bits[3] = (getSign(*src) << SIGN) | (exponent << START_SCALE);
}

/// @brief Получает степень экспоненты из s21_decimal
/// @param src
/// @return Степень десятичной экспоненты
int getExponent(s21_decimal src) { return (char)(src.bits[3] >> START_SCALE); }

/// @brief Получает знак из s21_decimal
/// @param src
/// @return 0 - '+', 1 - '-'
int getSign(s21_decimal src) { return (src.bits[3] >> SIGN) & 1; }

/// @brief Сравнивает два decArray по модулю
/// @param src1 Первое сравниваемое
/// @param src2 Второе сравниваемое
/// @return '1' - src1 больше, '-1' - src2 больше, '0' - src1 и src2 равны
int compareAbsDecArray(decArray src1, decArray src2) {
  for (int i = DEC_ARRAY_PART_MAX_SIZE - 1; i >= 0; i--)
    if (src1.integerPart[i] > src2.integerPart[i])
      return 1;
    else if (src1.integerPart[i] < src2.integerPart[i])
      return -1;

  for (int i = 0; i < DEC_ARRAY_PART_MAX_SIZE; i++)
    if (src1.floatPart[i] > src2.floatPart[i])
      return 1;
    else if (src1.floatPart[i] < src2.floatPart[i])
      return -1;

  return 0;
}

/// @brief Суммирует два decArray по модулю
/// @param src1 Первое слагаемое
/// @param src2 Второе слагаемое
/// @return Сумма
decArray decArrayAdd(decArray src1, decArray src2, int *exception) {
  decArray result = {0};

  int remainder = 0;
  for (int i = DEC_ARRAY_PART_MAX_SIZE - 1; i >= 0; i--) {
    result.floatPart[i] = remainder + src1.floatPart[i] + src2.floatPart[i];
    remainder = result.floatPart[i] / 10;
    result.floatPart[i] %= 10;
  }
  for (int i = 0; i < DEC_ARRAY_PART_MAX_SIZE; i++) {
    result.integerPart[i] =
        remainder + src1.integerPart[i] + src2.integerPart[i];
    remainder = result.integerPart[i] / 10;
    result.integerPart[i] %= 10;
  }
  if (remainder != 0 || compareAbsDecArray(result, maxDecArray()) == 1)
    *exception = 1;

  result.integerPartSize = countIntegerPartSize(result);
  result.floatPartSize = countFloatPartSize(result);

  return result;
}

/// @brief Переводит decArray в представление в виде
/// decArray десятичной мантиссы (целое число)
/// и значения экспоненты (степень десяти)
/// @param src
/// @return Значение экспоненты
int makeInteger(decArray *src) {
  int exp = src->floatPartSize;
  if (exp + src->integerPartSize > DEC_ARRAY_PART_MAX_SIZE)
    exp = DEC_ARRAY_PART_MAX_SIZE - src->integerPartSize;

  for (int i = src->integerPartSize - 1; i >= 0; i--)
    src->integerPart[i + exp] = src->integerPart[i];
  for (int i = 0; i < exp; i++)
    src->integerPart[exp - i - 1] = src->floatPart[i];
  for (int i = 0; i < DEC_ARRAY_PART_MAX_SIZE; i++) src->floatPart[i] = 0;

  src->integerPartSize = countIntegerPartSize(*src);
  src->floatPartSize = countFloatPartSize(*src);

  return exp;
}

/// @brief Рассчитывает произведение двух decArray
/// @param src1 Первый множитель
/// @param src2 Второй множитель
/// @return Произведение
decArray decArrayMul(decArray src1, decArray src2, int *exception) {
  decArray res = {0};

  if (src1.integerPartSize + src2.integerPartSize > DEC_ARRAY_PART_MAX_SIZE) {
    *exception = POSITIVE_INFINITY;
    return res;
  }

  int exp1 = makeInteger(&src1);
  int exp2 = makeInteger(&src2);

  int resArr[DEC_ARRAY_PART_MAX_SIZE * 2];
  for (int i = 0; i < DEC_ARRAY_PART_MAX_SIZE * 2; i++) resArr[i] = 0;

  for (int i = 0; i < DEC_ARRAY_PART_MAX_SIZE; i++) {
    res.integerPart[i] = 0;
    res.floatPart[i] = 0;
  }

  for (int j = 0; j < src2.integerPartSize; j++) {
    for (int i = 0; i < src1.integerPartSize; i++) {
      resArr[i + j] += src1.integerPart[i] * src2.integerPart[j];
    }
  }

  int remainder = 0;
  for (int i = 0; i < DEC_ARRAY_PART_MAX_SIZE * 2; i++) {
    resArr[i] += remainder;
    remainder = resArr[i] / 10;
    resArr[i] %= 10;
  }
  if (remainder != 0) {
    *exception =
        (src1.sign == src2.sign) ? POSITIVE_INFINITY : NEGATIVE_INFINITY;
    return res;
  }

  int exp = exp1 + exp2;
  for (int i = exp;
       i - exp < DEC_ARRAY_PART_MAX_SIZE && i < DEC_ARRAY_PART_MAX_SIZE * 2;
       i++) {
    res.integerPart[i - exp] = resArr[i];
  }
  for (int i = 0; i < DEC_ARRAY_PART_MAX_SIZE && exp - 1 - i >= 0; i++) {
    res.floatPart[i] = resArr[exp - 1 - i];
  }

  res.integerPartSize = countIntegerPartSize(res);
  res.floatPartSize = countFloatPartSize(res);
  res.sign = (src1.sign == src2.sign ? 0 : 1);

  return res;
}

/// @brief Делит два decArray
/// @param dividend Делимое
/// @param divisor Делитель
/// @return Частное
decArray decArrayDiv(decArray dividend, decArray divisor, int *exception) {
  if (isZero(divisor)) {
    *exception = DIVISION_BY_ZERO;
    decArray res = {0};
    return res;
  }
  if (isZero(dividend)) {
    *exception = OK;
    return dividend;
  }

  int expDividend = 0;
  while (dividend.integerPartSize != DEC_ARRAY_PART_MAX_SIZE) {
    multiplyByPow10(1, &dividend);
    expDividend++;
  }
  int expDivisor = 0;
  while (divisor.integerPartSize != DEC_ARRAY_PART_MAX_SIZE) {
    multiplyByPow10(1, &divisor);
    expDivisor++;
  }

  int resSign = (dividend.sign == divisor.sign ? 0 : 1);
  dividend.sign = 0;
  divisor.sign = 0;

  decArray one = {0};
  one.integerPart[0] = 1;
  one.integerPartSize = countIntegerPartSize(one);

  decArray res = {0};
  while (divisor.integerPartSize != 0 ||
         (divisor.floatPart[0] != 0 &&
          divisor.floatPartSize != DEC_ARRAY_PART_MAX_SIZE)) {
    int expRes = expDividend - expDivisor;

    decArray resPart = {0};
    while (compareAbsDecArray(dividend, divisor) != -1) {
      dividend = decArrayAbsSub(dividend, divisor);
      resPart = decArrayAbsAdd(resPart, one);
    }

    if (expRes > 0)
      divideByPow10(expRes, &resPart);
    else if (expRes < 0) {
      if (resPart.integerPartSize + (expRes < 0 ? -expRes : expRes) >
          DEC_ARRAY_PART_MAX_SIZE) {
        *exception = (resSign == 0 ? 1 : 2);
        break;
      }
      multiplyByPow10(-expRes, &resPart);
      if (compareAbsDecArray(resPart, maxDecArray()) == 1) {
        *exception = (resSign == 0 ? 1 : 2);
        break;
      }
    }
    res = decArrayAbsAdd(res, resPart);

    divideByPow10(1, &divisor);
    expDivisor--;
  }

  if (compareAbsDecArray(res, maxDecArray()) == 1) {
    *exception = (resSign == 0 ? 1 : 2);
    decArray temp = {0};
    return temp;
  }

  res.sign = resSign;
  return res;
}

/// @brief Суммирует два decArray по модулю
/// @param src1 Первое слагаемое
/// @param src2 Второе слагаемое
/// @return Сумма
decArray decArrayAbsAdd(decArray src1, decArray src2) {
  decArray result = {0};

  int remainder = 0;
  for (int i = DEC_ARRAY_PART_MAX_SIZE - 1; i >= 0; i--) {
    result.floatPart[i] = remainder + src1.floatPart[i] + src2.floatPart[i];
    remainder = result.floatPart[i] / 10;
    result.floatPart[i] %= 10;
  }
  for (int i = 0; i < DEC_ARRAY_PART_MAX_SIZE; i++) {
    result.integerPart[i] =
        remainder + src1.integerPart[i] + src2.integerPart[i];
    remainder = result.integerPart[i] / 10;
    result.integerPart[i] %= 10;
  }

  result.integerPartSize = countIntegerPartSize(result);
  result.floatPartSize = countFloatPartSize(result);

  return result;
}

/// @brief Вычитает один decArray из другого по модулю
/// @param src1 Уменьшаемое (src1 > src2)
/// @param src2 Вычитатель
/// @return Разность
decArray decArrayAbsSub(decArray src1, decArray src2) {
  decArray result = {0};

  for (int i = 0; i < DEC_ARRAY_PART_MAX_SIZE; i++)
    result.integerPart[i] = src1.integerPart[i] - src2.integerPart[i];
  for (int i = 0; i < DEC_ARRAY_PART_MAX_SIZE; i++)
    result.floatPart[i] = src1.floatPart[i] - src2.floatPart[i];

  bool flag = false;
  while (!flag) {
    flag = true;
    for (int i = DEC_ARRAY_PART_MAX_SIZE - 1; i > 0; i--)
      if (result.floatPart[i] < 0) {
        flag = false;
        result.floatPart[i - 1] -= 1;
        result.floatPart[i] += 10;
      }
    if (result.floatPart[0] < 0) {
      flag = false;
      result.integerPart[0] -= 1;
      result.floatPart[0] += 10;
    }
    for (int i = 0; i < DEC_ARRAY_PART_MAX_SIZE; i++) {
      if (result.integerPart[i] < 0) {
        flag = false;
        result.integerPart[i + 1] -= 1;
        result.integerPart[i] += 10;
      }
    }
  }
  result.integerPartSize = countIntegerPartSize(result);
  result.floatPartSize = countFloatPartSize(result);

  return result;
}

/// @brief Умножает decArray на степень десяти
/// @param exp Степень десяти
/// @param src Указатель на decArray
void multiplyByPow10(int exp, decArray *src) {
  if (exp == 0) return;

  for (int i = DEC_ARRAY_PART_MAX_SIZE - 1; i >= exp; i--) {
    src->integerPart[i] = src->integerPart[i - exp];
    src->integerPart[i - exp] = 0;
  }
  for (int i = exp - 1; i >= 0; i--) {
    src->integerPart[i] = src->floatPart[exp - i - 1];
    src->floatPart[exp - i - 1] = 0;
  }
  for (int i = exp - 1; i < DEC_ARRAY_PART_MAX_SIZE - exp; i++) {
    src->floatPart[i] = src->floatPart[i + exp];
    src->floatPart[i + exp] = 0;
  }

  src->integerPartSize = countIntegerPartSize(*src);
  src->floatPartSize = countFloatPartSize(*src);
}

/// @brief Проверяет, является ли decArray представлением нуля
/// @param src
/// @return Результат
int isZero(decArray src) {
  for (int i = 0; i < DEC_ARRAY_PART_MAX_SIZE; i++)
    if (src.floatPart[i] != 0 || src.integerPart[i] != 0) return false;
  return true;
}

/// @brief Делит decArray на степень десяти
/// @param exp Степень десяти
/// @param src Указатель на decArray
void divideByPow10(int exp, decArray *src) {
  if (exp <= 0 || exp >= DEC_ARRAY_PART_MAX_SIZE) return;

  for (int i = DEC_ARRAY_PART_MAX_SIZE - 1; i >= exp && i - exp >= 0; i--) {
    src->floatPart[i] = src->floatPart[i - exp];
    src->floatPart[i - exp] = 0;
  }
  for (int i = exp - 1;
       i >= 0 && exp - i - 1 >= 0 && exp - i - 1 < DEC_ARRAY_PART_MAX_SIZE;
       i--) {
    src->floatPart[i] = src->integerPart[exp - i - 1];
    src->integerPart[exp - i - 1] = 0;
  }
  for (int i = 0;
       i < DEC_ARRAY_PART_MAX_SIZE && i + exp < DEC_ARRAY_PART_MAX_SIZE; i++) {
    src->integerPart[i] = src->integerPart[i + exp];
    src->integerPart[i + exp] = 0;
  }

  src->integerPartSize = countIntegerPartSize(*src);
  src->floatPartSize = countFloatPartSize(*src);
}

/// @brief Возвращает результат банковского округления
/// @param lastBit Значение десятичного разряда до которого происходит
/// округление
/// @param bitAfterLast Значение разряда, следующего за lastBit
/// @return Округленное значение разряда lastBit
int rounding(int lastBit, int bitAfterLast) {
  if (bitAfterLast == 5) return (lastBit % 2 == 0 ? lastBit : lastBit + 1);
  if (bitAfterLast < 5) return lastBit;
  return lastBit + 1;
}

/// @brief Округляет decArray до 29 знаков в соответствии с правилами
/// банковского округления
/// @param src
/// @return Округленный decArray
decArray bankRoundDecArray(decArray src) {
  int i = DEC_ARRAY_PART_MAX_SIZE - src.integerPartSize;
  if (i == 0)
    src.integerPart[0] = rounding(src.integerPart[0], src.floatPart[0]);
  else if (i != DEC_ARRAY_PART_MAX_SIZE)
    src.floatPart[i - 1] = rounding(src.floatPart[i - 1], src.floatPart[i]);
  for (; i < DEC_ARRAY_PART_MAX_SIZE; i++) src.floatPart[i] = 0;

  src.integerPartSize = countIntegerPartSize(src);
  src.floatPartSize = countFloatPartSize(src);
  return src;
}

/// @brief Возвращает максимальное значение s21_decimal в формате decArray
/// @return decArray представление максимального числа, которое можно записать в
/// s21_decimal
decArray maxDecArray() {
  s21_decimal maxDecimal = {0};
  maxDecimal.bits[0] = 4294967295U;  // UINT32_MAX
  maxDecimal.bits[1] = 4294967295U;
  maxDecimal.bits[2] = 4294967295U;
  maxDecimal.bits[3] = 0;

  return getDecimalAsDecArray(maxDecimal);
}
