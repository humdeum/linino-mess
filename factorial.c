#include <stdio.h>
#include <stdlib.h>

long fact(long n)
{
	if (n==1)
		return 1;
	else
		return n * fact(n-1);
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
		printf("%ld\n", fact(i));
	return 0;
}
