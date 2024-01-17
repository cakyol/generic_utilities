
#include <stdio.h>

struct test_s {
	char byte1;
	char byte2;
	short s;
} ;

#define null	((void*)0)

#define XXX	((struct test_s *) null)->byte1


int main (int argc, char *argv[])
{

	XXX = (char) 1;

	return 0;
}
