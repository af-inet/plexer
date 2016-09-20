#include <time.h>
#include <string.h>

#ifndef NOCOLOR
	#define COLOR_RED   "\033[38;5;1m"
	#define COLOR_GREEN "\033[38;5;10m"
	#define COLOR_WHITE "\033[38;5;15m"
	#define COLOR_END   "\e[0m"
#else
	#define COLOR_RED   ""
	#define COLOR_GREEN ""
	#define COLOR_WHITE ""
	#define COLOR_END   ""
#endif

#define CTEST_ASSERT(exp)\
printf(COLOR_WHITE "%s:%-4d ", __FILE__, __LINE__);\
if(exp)\
	printf("%s ", (COLOR_GREEN "PASS " COLOR_END));\
else\
	printf("%s ", (COLOR_RED "FAIL " COLOR_END));\
printf(COLOR_WHITE "%s   (%d)" COLOR_END "\n", #exp, (exp));\

#define CTEST_EQ(s1, s2)\
printf(COLOR_WHITE "%s:%-4d ", __FILE__, __LINE__);\
if(exp)\
	printf("%s ", (COLOR_GREEN "PASS " COLOR_END));\
else\
	printf("%s ", (COLOR_RED "FAIL " COLOR_END));\
printf(COLOR_WHITE "%s == %s" COLOR_END "\n", s1, s2);\

#define CTEST_STREQ(s1, s2)\
printf(COLOR_WHITE "%s:%-4d ", __FILE__, __LINE__);\
if(strcmp(s1, s2) == 0)\
	printf("%s ", (COLOR_GREEN "PASS " COLOR_END));\
else\
	printf("%s ", (COLOR_RED "FAIL " COLOR_END));\
printf(COLOR_WHITE "%s == %s" COLOR_END "\n", s1, s2);\

#define CTEST_TIME(name, body)\
{ \
clock_t t; \
double time_taken; \
t = clock(); \
body; \
t = clock() - t; \
time_taken = ((double)t)/CLOCKS_PER_SEC; \
printf(COLOR_WHITE "%s:%-4d %s %.4f seconds\n" COLOR_END, __FILE__, __LINE__, #name, time_taken); \
} \

