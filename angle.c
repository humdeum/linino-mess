#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>

#define ALEN 30
#define ENABLE 1
#define DISABLE 0
#define MAXSTEP 50
#define INTERVAL 100000
#define RANGE 1023

struct adc
{
	char fn[ALEN];
	FILE *fp;
	int pin;
	double val;
	char value[ALEN];
	char label[ALEN];
};

int adc_open(struct adc *adc);
int enable_adc(int status);
int adc_read(struct adc *adc);

char *sbi = "/sys/bus/iio/devices/iio:device0/";
int debug = 1;

int main()
{
	enable_adc(ENABLE);
	
	struct adc cos;
	cos.pin = 0;
	strcat(cos.label, "cos");
	adc_open(&cos);
	
	struct adc sin;
	sin.pin = 1;
	strcat(sin.label, "sin");
	adc_open(&sin);
	
	int i;
	for (i=0; 1<MAXSTEP; i++)
	{
		adc_read(&cos);
		adc_read(&sin);

		printf("cos=%f, sin=%f", cos.val, sin.val);
		printf(", angle=%f\n", atan2(sin.val, cos.val));

		usleep(INTERVAL);
	}
	
	enable_adc(DISABLE);
}

int adc_read(struct adc *adc)
{
	fread(adc->value, sizeof(char), ALEN-1, adc->fp);
	adc->val = (double)(atoi(adc->value)-512)/RANGE;
	rewind(adc->fp);
	return 0;
}

int adc_open(struct adc *adc)
{
	strcpy(adc->fn, sbi);
	char voltagestr[ALEN];
	sprintf(voltagestr, "in_voltage_A%d_raw", adc->pin);
	strcat(adc->fn, voltagestr);
	if (debug) printf("Debug: %s.fn=%s\n", adc->label, adc->fn);
	adc->fp = fopen(adc->fn, "r");
	return 0;
}

int enable_adc(int status)
{
	struct adc enable;
	strcpy(enable.fn, sbi);
	strcat(enable.fn, "enable");
	enable.fp = fopen(enable.fn, "w");
	fprintf(enable.fp ,"%d", status);
	fflush(enable.fp);
	fclose(enable.fp);
	return 0;
}
