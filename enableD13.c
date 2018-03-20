#include <stdio.h>

int main(void)
{
	FILE *unexportPtr = fopen("/sys/class/gpio/export", "w");
	fprintf(unexportPtr, "115");
	fclose(unexportPtr);

	FILE *valuePtr = fopen("/sys/class/gpio/D13/value", "w");
	fprintf(valuePtr, "1");
	fclose(valuePtr);

	return 0;
}
