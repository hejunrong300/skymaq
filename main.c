

#include "add.h"
#include "prb0400Lib.h"
#include <stdio.h>

int main()
{
	printf("hello world!!!\n");
	int a = 4, b = 5;
	add(a, b);
	a++;
	b++;
	add(a, b);
	// ret = rioInit();
	return 0;
}