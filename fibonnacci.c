#include <stdio.h>
#include <stdlib.h>

long fib(long n)
{
	if (n==1 || n==2)
		return 1;
	else
		return fib(n-1) + fib(n-2);
}

int main(int argc, char **argv)
{
	if (argv[1]==NULL)
	{
		printf("E: Please supply a number.");
		return 1;
	}
	long n = strtol(argv[1], NULL, 10);
	for (int i=1; i<=n; i++)
		printf("%ld\n", fib(i));
	return 0;
}
