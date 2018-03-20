#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#define D2 117
#define D3 116
#define D4 120
#define D5 114
#define D6 123
#define D8 104
#define D9 105
#define D10 106
#define D11 107
#define D12 122
#define D13 115

//void setv(FILE *fp, int v, int delay);

int main (int argc, char **argv) {
	char *export = "13";
	char *interval = "1000000";
	char *freq = "1";
	int index,c,pin;
	int debug = 0;

	opterr = 0;

 	while ((c = getopt (argc, argv, "d e:s:f:")) != -1)
		switch (c) {
		case 'd':
			debug = 1;
			break;
 		case 'e':
			export = optarg;
			break;
		case 's':
			interval = optarg;
			break;
		case 'f':
			freq = optarg;
			break;
		case '?':
			if ((optopt == 'e') || (optopt == 'f'))
				fprintf (stderr, "Option -%c requires an argument.\n", optopt);
			else if (isprint (optopt))
				fprintf (stderr, "Unknown option `-%c'.\n", optopt);
			else
				fprintf (stderr,
				"Unknown option character `\\x%x'.\n",
				optopt);
			return 1;
		default:
			abort ();
		}

	if (debug)
		printf ("export = %s, interval = %d, freq = %d\n",
			export, atoi(interval), atoi(freq));
 	for (index = optind; index < argc; index++)
 		printf ("Non-option argument %s\n", argv[index]);
	
	switch (atoi(export)) {
	case 2:
		pin = D2;
		break;
	case 3:
		pin = D3;
		break;
	case 4:
		pin = D4;
		break;
	case 5:
		pin = D5;
		break;
	case 6:
		pin = D6;
		break;
	case 8:
		pin = D8;
		break;
	case 9:
		pin = D9;
		break;
	case 10:
		pin = D10;
		break;
	case 11:
		pin = D11;
		break;
	case 12:
		pin = D12;
		break;
	case 13:
		pin = D13;
		break;
	default:
		printf("Invalid PIN number\n");
		return 1;
		abort ();
	}
	
	if (debug)
		printf("pin = %d\n", pin);
	
	char *gpio = "/sys/class/gpio/";
	char pinPath[20];
	strcpy(pinPath, gpio);
	strcat(pinPath, "D");
	strcat(pinPath, export); // Possible vulnerability
	
	char exportfn[20];
	strcpy(exportfn, gpio);
	strcat(exportfn, "export");
	if (debug)
		printf("%s\n", exportfn);

	FILE *exportfp = fopen(exportfn, "w");
	fprintf(exportfp, "%d", pin);
	fclose(exportfp);

	char directionfn[30];
	strcpy(directionfn, pinPath);
	strcat(directionfn, "/direction");
	if (debug)
		printf("%s\n", directionfn);

	FILE *directionfp = fopen(directionfn, "w");
	fprintf(directionfp, "out");
	fclose(directionfp);

	char valuefn[30] = "";
	strcat(valuefn, pinPath);
	strcat(valuefn, "/value");

	if (debug)
		printf("%s\n", valuefn);
	FILE *valuefp = fopen(valuefn, "w");

	volatile int blink;
	void handleCtrlC (int sig) {
		blink = 0;
	}
	blink = 1;
	signal(SIGINT, handleCtrlC);

	while (blink) {
	/*
	setv(valuefp, 1, atoi(interval));
	setv(valuefp, 0, atoi(interval));
	*/
		fprintf(valuefp, "%d", 1);
		fflush(valuefp);
		if (debug) {
			printf("%d", 1);
			fflush(stdout);
		}
		usleep(atoi(interval));

		fprintf(valuefp, "%d", 0);
		fflush(valuefp);
		if (debug) {
			printf("%d", 0);
			fflush(stdout);
		}
		usleep(atoi(interval));
	}
	printf("\n");
	fclose(valuefp);

	char unexportfn[20];
	strcpy(unexportfn, gpio);
	strcat(unexportfn, "unexport");
	printf("%s\n", unexportfn);

	FILE *unexportfp = fopen(unexportfn, "w");
	fprintf(unexportfp,"%d",pin);
	fclose(unexportfp);
	
 	return 0;
}
/*
void setv(FILE *fp,int v, int delay){
	fprintf(fp, "%d", v);
	fflush(fp);
	printf("%d", v);
	fflush(stdout);
	usleep(delay);
}
*/
