#include <stdio.h>

int main(void)
{
	FILE *valuePtr = fopen("/sys/class/gpio/D13/value", "w");
	fprintf(valuePtr, "0");
	fclose(valuePtr);

	FILE *unexportPtr = fopen("/sys/class/gpio/unexport", "w");
	fprintf(unexportPtr, "115");
	fclose(valuePtr);
	return 0;
}
