#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#define ALEN 30

char *scg = "/sys/class/gpio/";
void reset_reg (FILE *resetfp, FILE *f_enfp, int debug);
char *value_prep(char *returnfn, char *exportPin, int debug);
int map(int p);

int main(int argc, char **argv) {
	char *export = "13";
	char *interval = "100000";
	char *freq = NULL;
	char *transmit = NULL;
	char *enable = NULL;
	char *reset = NULL;
	char *direction = NULL;
	char *clockwise = "0";
	char *on = NULL;
	int index,c,pin;
	int debug = 0;

	opterr = 0;

 	while ((c = getopt (argc, argv, "dc:e:r:s:f:t:w:o:")) != -1)
		switch (c) {
		case 'o':
			on = optarg; // on pin number
		case 'w':
			clockwise = optarg; // dir pin value
			break;
		case 'c':
			direction = optarg; // dir pin number
			break;
		case 'd':
			debug = 1;
			break;
 		case 'e':
			export = optarg; // f_en pin number for transmit
			break;
		case 's':
			interval = optarg; // usleep value
			break;
		case 'r':
			reset = optarg; // reset pin number for transmit sync
			break;
		case 't':
			transmit = optarg; // transmit value string
			break;
		case 'f': // freq pin number to transmit with
			enable = optarg;
			break;
		case '?':
			if ((optopt=='e') || (optopt=='f') || (optopt=='t') || (optopt=='s') || (optopt=='r') || (optopt=='c') || (optopt=='o'))
				fprintf (stderr, "Option -%c requires an argument.\n", optopt);
			else if (isprint (optopt))
				fprintf (stderr, "Unknown option `-%c'.\n", optopt);
			else
				fprintf (stderr,
				"Unknown option character `\\x%x'.\n",
				optopt);
			return 1;
		default:
			abort();
		}
	if (debug)
		printf ("on = %s, export = %s, interval = %s, reset = %s, transmit = %s, enable = %s, direction = %s\n",
			on, export, interval, reset, transmit, enable, direction);
 	for (index = optind; index < argc; index++)
 		printf ("Non-option argument %s\n", argv[index]);
	
	int valuePin = map(atoi(export));
	int enablePin = (enable == NULL) ? 0 : map(atoi(enable));
	int resetPin = (reset == NULL) ? 0 : map(atoi(reset));
	int dirPin = (direction == NULL) ? 0 : map(atoi(direction));
	int delay = atoi(interval);
	int onPin = (on == NULL) ? 0 : map(atoi(on));
	
	char valuefn[ALEN] = "";
	value_prep(valuefn, export, debug);
	FILE *valuefp = fopen(valuefn, "w");
	
	char unexportfn[ALEN];
	strcpy(unexportfn, scg);
	strcat(unexportfn, "unexport");
	FILE *unexportfp = fopen(unexportfn, "w");
	
	if (direction != NULL) {
		char dirfn[ALEN];
		value_prep(dirfn, direction, debug);
		FILE *dirfp = fopen(dirfn, "w");
		if (debug)
			printf("%s > %s\n", clockwise, dirfn);
		fprintf(dirfp, "%s", clockwise);
		fflush(dirfp);
		if (debug)
			printf("%d > %s\n", dirPin, unexportfn);
		fprintf(unexportfp, "%d", dirPin);
		fflush(unexportfp);
	}

	volatile int blink;
	void handleCtrlC (int sig) {
		blink = 0;
	}
	blink = 1;
	signal(SIGINT, handleCtrlC);

	if (transmit == NULL) { // blink
		if (!dirPin) {
			while (blink) {
				fprintf(valuefp, "%d", 1);
				fflush(valuefp);
				if (debug) {
					printf("%d", 1);
					fflush(stdout);
				}
				usleep(delay);
				
				fprintf(valuefp, "%d", 0);
				fflush(valuefp);
				if (debug) {
					printf("%d", 0);
					fflush(stdout);
				}
				usleep(delay);
			}
			printf("\n");
		}
	} else { // transmit
		if (onPin == 0) {
			fprintf(stderr, "Use -o to select the enable pin\n");
			return 1;
			abort();
		}
		char onfn[ALEN] = "";
		value_prep(onfn, on, debug);
		FILE *onfp = fopen(onfn, "w");
		if (debug)
			printf("%d > %s\n", 1, onfn);
		fprintf(onfp, "%d", 1);
		fflush(onfp);

		if (debug)
			printf("Transmit\n");

		char enablefn[ALEN] = "";
		value_prep(enablefn, enable, debug);	
		FILE *enablefp = fopen(enablefn, "w");
		
		char unexportfn[ALEN];
		strcpy(unexportfn, scg);
		strcat(unexportfn, "unexport");
		FILE *unexportfp = fopen(unexportfn, "w");

		if (reset != NULL) {
			char resetfn[ALEN] = "";
			value_prep(resetfn, reset, debug);
			FILE *resetfp = fopen(resetfn, "w");

			if (debug)
				printf("%d > %s\n", 1, resetfn);
			fprintf(resetfp, "%d", 1);
			fflush(resetfp);
			usleep(delay);
			
			if (debug)
				printf("%d > %s\n", 1, enablefn);
			fprintf(enablefp, "%d", 1);
			fflush(enablefp);
			usleep(delay);
			
			if (debug)
				printf("%d > %s\n", 0, enablefn);
			fprintf(enablefp, "%d", 0);
			fflush(enablefp);
			usleep(delay);
			
			if (debug)
				printf("%d > %s\n", 0, resetfn);
			fprintf(resetfp, "%d", 0);	
			fflush(resetfp);
			usleep(delay);

			fclose(resetfp);
			if (debug)
				printf("%d > %s\n", resetPin, unexportfn);
			fprintf(unexportfp, "%d", resetPin);
			fflush(unexportfp);
		}

		int i, j = strlen(transmit);
		for (i=0; i<j; i++) {
			if (debug) {
				printf(" (%d) ", transmit[i] - '0');
				fflush(stdout);
			}
			fprintf(valuefp, "%d", transmit[i] - '0');
			fflush(valuefp);
			usleep(delay);

			if (debug) {
				printf("%d", 0);
				fflush(stdout);
			}
			fprintf(enablefp, "%d", 1);
			fflush(enablefp);
			usleep(delay);

			if (debug) {
				printf("%d", 1);
				fflush(stdout);
			}
			fprintf(enablefp, "%d", 0);
			fflush(enablefp);
			usleep(delay);
		}
		printf("\n");
		fclose(enablefp);

		/*
		strcpy(unexportfn, scg);
		strcat(unexportfn, "unexport");
		*/
		if (debug)
			printf("%d > %s\n", enablePin, unexportfn);
		
		fprintf(unexportfp,"%d", enablePin);
		//fclose(unexportfp);
	}
	fclose(valuefp);

	/*
	strcpy(unexportfn, scg);
	strcat(unexportfn, "unexport");
	*/
	if (debug)
		printf("%d > %s\n",  valuePin, unexportfn);
	fprintf(unexportfp, "%d", valuePin);
	fflush(unexportfp);

	if (debug)
		printf("%d > %s\n",  onPin, unexportfn);
	fprintf(unexportfp, "%d", onPin);
	fflush(unexportfp);

	fclose(unexportfp);

 	return 0;
}

void reset_reg(FILE *resetfp, FILE *f_enfp, int debug) {
	if (debug)
		printf("Reset\n");
	fprintf(resetfp, "%d", 1);
	fprintf(f_enfp, "%d", 1);
	fprintf(f_enfp, "%d", 0);
	fprintf(resetfp, "%d", 0);	
}

char *value_prep(char *returnfn, char *exportPin, int debug) {

	int pin = map(atoi(exportPin));

	char pinPath[ALEN];
	strcpy(pinPath, scg);
	strcat(pinPath, "D");
	strcat(pinPath, exportPin); // bad
	
	char exportfn[ALEN];
	strcpy(exportfn, scg);
	strcat(exportfn, "export");
	if (debug)
		printf("%d > %s\n", pin, exportfn);

	FILE *exportfp = fopen(exportfn, "w");
	fprintf(exportfp, "%d", pin);
	fclose(exportfp);

	char directionfn[ALEN];
	strcpy(directionfn, pinPath);
	strcat(directionfn, "/direction");
	if (debug)
		printf("out > %s\n", directionfn);

	FILE *directionfp = fopen(directionfn, "w");
	fprintf(directionfp, "out");
	fclose(directionfp);

	char valuefn[ALEN] = "";
	strcat(valuefn, pinPath);
	strcat(valuefn, "/value");
	
	if (debug)
		printf("\t%s\n", valuefn);
	strcpy(returnfn, valuefn);

	return 0;
}
	

int map(int p) {
	int pin;

	switch (p) {
	case 2:
		pin = 117;
		break;
	case 3:
		pin = 116;
		break;
	case 4:
		pin = 120;
		break;
	case 5:
		pin = 114;
		break;
	case 6:
		pin = 123;
		break;
	case 8:
		pin = 104;
		break;
	case 9:
		pin = 105;
		break;
	case 10:
		pin = 106;
		break;
	case 11:
		pin = 107;
		break;
	case 12:
		pin = 122;
		break;
	case 13:
		pin = 115;
		break;
	default:
		printf("Invalid PIN number\n");
		abort();
	}
	return pin;
}
