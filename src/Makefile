CP := gcc
CFLAGS := -Wall -Werror -Wextra -std=c11
GCOVFLAGS := -L. --coverage  -ls21_decimal
OS = $(shell uname)
ifeq ($(OS), Linux)
	CHECK_FLAGS = $(shell pkg-config --libs check) -lm
else
	CHECK_FLAGS = -lcheck
endif
SRC := $(wildcard s21_*.c)
HDR := $(wildcard s21_*.h)
OBJ := $(patsubst %.c,%.o, ${SRC})

all: clean s21_decimal.a test 

s21_decimal.o:
	$(CP) $(CFLAGS) -c $(SRC)

s21_decimal.a: s21_decimal.o
	ar rcs libs21_decimal.a $(OBJ)
	ranlib libs21_decimal.a
	cp libs21_decimal.a s21_decimal.a

test: s21_decimal.a
	$(CP) $(CFLAGS) -o test test.c $^ $(CHECK_FLAGS)
	./test

gcov_report: s21_decimal.a
	$(CP) $(CFLAGS) $(SRC) test.c libs21_decimal.a $(CHECK_FLAGS) $(GCOVFLAGS) -o $@ 
	./gcov_report
	lcov -t gcov_report -o rep.info -c -d .
	genhtml -o report rep.info
	open ./report/index.html
	rm -rf *.gcda *.gcno *.info

check:
	cp ../materials/linters/.clang-format .
	clang-format -n *.c *.h
	make test
	CK_FORK=no leaks --atExit -- ./test

clean:
	rm -rf test *.a *.o *.gcno *.gcda *.info test_output gcov_report report
