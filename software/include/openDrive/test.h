/*

Add a section like this to the module you want to have tested

#ifdef TEST_MODE
TEST_PREFIX
	RUN_TEST(unequal,{1==0;},0);
	RUN_TEST(must_fail,{1==0;},1);
	RUN_TEST(equal,{1==1;},1);
TEST_SUFFIX
#endif 

*/ 

#ifndef TEST_H
#define TEST_H
#include <stdio.h>

static unsigned int testcase_count = 0;
static unsigned int testcase_fail = 0;

#define RUN_TEST(name,block,expect) \
	do { \
		int ___rc;\
		testcase_count++; \
		___rc = (block); \
		if (___rc != expect) { \
			printf("Testcase %s in File %s Line %d failed: Result %d, expect %s\n", \
				#name,__FILE__,__LINE__,___rc,#expect); \
			testcase_fail ++; \
		} \
	} while(0)

#define TEST_PREFIX \
	int main(int argc, char * argv) { \
		printf("Performing Test cases from File: %s\n",__FILE__);

#define TEST_SUFFIX \
		printf("%d out of %d test cases failed\n",testcase_fail,testcase_count); \
		return 0; \
	} 
#endif /* TEST_H */

