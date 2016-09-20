#ifndef CTEST_H
#define CTEST_H

#include <time.h>
#include <string.h>

static int __ctest_ret = 0;

#ifndef NOCOLOR
	#define C_RED   "\033[38;5;1m"
	#define C_GREEN "\033[38;5;10m"
	#define C_WHITE "\033[38;5;15m"
	#define C_END   "\e[0m"
#else
	#define C_RED   ""
	#define C_GREEN ""
	#define C_WHITE ""
	#define C_END   ""
#endif

#define CTEST_RETURN() \
return __ctest_ret;

#define HL(str, color) (color str C_END)

#define CTEST_PASS() \
printf("%s ", (HL("PASS ", C_GREEN)))

#define CTEST_FAIL() \
printf("%s ", (HL("FAIL ", C_RED))); \
__ctest_ret = 1

#define CTEST_SUB(cb) \
printf("\n-- %4s --\n", #cb); \
cb()

#define CTEST_ASSERT(exp) \
printf( HL("%s:%-4d ", C_WHITE), __FILE__, __LINE__); \
if(exp) { CTEST_PASS(); } else { CTEST_FAIL(); } \
printf( HL("%s\n", C_WHITE), #exp); \

#define CTEST_EQ(s1, s2) \
printf( HL("%s:%-4d ", C_WHITE), __FILE__, __LINE__); \
if(exp) { CTEST_PASS(); } else { CTEST_FAIL(); } \
printf( HL("%s == %s\n", C_WHITE), s1, s2); \

#define CTEST_STREQ(s1, s2) \
printf( HL("%s:%-4d ", C_WHITE), __FILE__, __LINE__); \
if(strcmp(s1, s2) == 0) { CTEST_PASS(); } else { CTEST_FAIL(); } \
printf( HL("%s == %s\n", C_WHITE), s1, s2); \

#define CTEST_TIME(name, body) \
{ \
clock_t t; \
double time_taken; \
t = clock(); \
body; \
t = clock() - t; \
time_taken = ((double)t)/CLOCKS_PER_SEC; \
printf( HL("%s:%-4d %s %.4f seconds\n", C_WHITE), __FILE__, __LINE__, #name, time_taken); \
} \

#endif /* CTEST_H */
