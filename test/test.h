#include <time.h>
#include <string.h>

#ifndef NOCOLOR
	#define COLOR_RED   "\033[38;5;1m"
	#define COLOR_GREEN "\033[38;5;10m"
	#define COLOR_WHITE "\033[38;5;15m"
#else
	#define COLOR_RED   ""
	#define COLOR_GREEN ""
	#define COLOR_WHITE ""
#endif

#define CTEST_ASSERT(exp)\
printf(COLOR_WHITE "%s:%-4d %s ", __FILE__, __LINE__, #exp);\
if(exp)\
	printf("%s", (COLOR_GREEN "PASS\n") );\
else\
	printf("%s", (COLOR_RED "FAIL\n") );\

#define CTEST_EQ(s1, s2)\
printf(COLOR_WHITE "%s:%-4d %s == %s ", __FILE__, __LINE__, #s1, #s2);\
if(s1==s2)\
	printf("%s", (COLOR_GREEN "PASS\n") );\
else\
	printf("%s", (COLOR_RED "FAIL\n") );\

#define CTEST_NEQ(s1, s2)\
printf(COLOR_WHITE "%s:%-4d %s != %s ", __FILE__, __LINE__, #s1, #s2);\
if(s1!=s2)\
	printf("%s", (COLOR_GREEN "PASS\n") );\
else\
	printf("%s", (COLOR_RED "FAIL\n") );\

#define CTEST_STREQ(s1, s2)\
printf(COLOR_WHITE "%s:%-4d %s == %s ", __FILE__, __LINE__, #s1, #s2);\
if(strcmp(s1,s2)==0)\
	printf("%s", (COLOR_GREEN "PASS\n") );\
else\
	printf("%s", (COLOR_RED "FAIL\n") );\


#define CTEST_TIME(name, body)\
{ \
clock_t t; \
double time_taken; \
t = clock(); \
body; \
t = clock() - t; \
time_taken = ((double)t)/CLOCKS_PER_SEC; \
printf(COLOR_WHITE "%s:%-4d %s %.4f seconds\n", __FILE__, __LINE__, #name, time_taken); \
} \

