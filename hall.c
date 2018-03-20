#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>

#define MAXSAMPLES 1000
#define ALEN 30
#define NEGEDGE 0
#define POSEDGE 1

typedef struct avg
{
	char *label;
	uint64_t newt;
	uint64_t oldt;
	uint64_t sample[MAXSAMPLES];
	uint64_t avg;
	long iter;
	int wsize;
	int fall;
} avg_t;

struct pin
{
	char fn[ALEN];
	FILE *fp;
	char Pnum[ALEN]; // external number
	char pnum[ALEN]; // internal number
};

struct arg
{
	int debug;
	int poll;
	int wlen;
	int edgew;
	int lpin;
};

uint64_t utcalc(struct timespec *ts);
void shift_sample(struct avg *t, struct arg *a);
int parse_opts(struct arg *a, int argc, char **argv);
int map_gpio(int p);
int gpio_prep(struct pin *p, struct arg *a);
int gpio_cleanup(struct pin *p, struct arg *a);
int avg_recw(struct avg *t, uint64_t timer, struct arg *a);
int edge_detect
(
	struct avg *t,
	int *val,
	uint64_t timer,
	struct arg *a
);
int avg_both(
	struct avg *b,
	struct avg *p,
	struct avg *n,
	struct arg *a
);

int main(int argc, char **argv)
{
	struct arg args = {0};
	parse_opts(&args, argc, argv);
	
	struct pin hall;
	gpio_prep(&hall, &args);
	
	volatile int run = 1;
	void handleCtrlC(int sig)
	{
		run = 0;
		if (args.debug) printf("\nrun=%d\n", run);
	}
	signal(SIGINT, handleCtrlC);
	
	avg_t neg={0}, pos={0}, both={0};
	neg.label = "neg";
	pos.label = "pos";
	both.label = "both";
	neg.fall = NEGEDGE;
	pos.fall = POSEDGE;
	
	uint64_t timer;
	struct timespec ts;
	clock_getres(CLOCK_MONOTONIC, &ts);
	if (args.debug)
		printf("Debug: clock resolution is %ld ns\n", ts.tv_nsec);
	clock_gettime(CLOCK_MONOTONIC, &ts);
	
	int i;
	char value[ALEN];
	int val[args.wlen];
	for (i=0; i<args.wlen; i++) val[i] = 0;
	
	if (args.debug) printf("Debug: while (%d)\n", run);
	while (run)
	{
		usleep(args.poll);
		clock_gettime(CLOCK_MONOTONIC, &ts);
		timer = utcalc(&ts);
		
		fread(value, sizeof(char), ALEN-1, hall.fp);
		for (i=args.wlen-1; i>0; i--) val[i] = val[i-1];
		val[0] = atoi(value);
		
		edge_detect(&pos, val, timer, &args);
		edge_detect(&neg, val, timer, &args);
		avg_both(&both, &pos, &neg, &args);
		
		// estimate time for angle from rev time both.avg
		
		rewind(hall.fp);
	}
	gpio_cleanup(&hall, &args);
	return 0;
}

int avg_both(
	struct avg *b,
	struct avg *p,
	struct avg *n,
	struct arg *a
)
{
	int i;
	if ((p->iter == n->iter)
		&&(b->iter < n->iter)&&(n->iter > 0)
		&&(b->iter < p->iter)&&(p->iter > 0))
	{
		shift_sample(b, a);
		b->iter++;
		b->avg = 0;
		b->wsize = (a->wlen-1 < b->iter)
			? a->wlen : b->iter;
		for (i=0; i<a->wlen; i++)
		{
			if ((p->sample[i] != 0)&&(n->sample[i] != 0))
			{
				b->sample[i] = (p->sample[i] + n->sample[i])/2;
				b->avg = b->avg + b->sample[i] / b->wsize;
			}
			if (a->debug)
			{
				printf("%s.sample[%d]=%" PRIu64, 
					b->label, i, b->sample[i]);
				if (i != a->wlen-1) printf("\n");
			}
		}
		if (a->debug)
		{
			printf(", %s.avg=%" PRIu64, b->label, b->avg);
			printf(", %s.wsize=%d\n", b->label, b->wsize);
		}
	}
	return 0;
}

int map_gpio(int p)
{ // lininoIO
	int pin;
	switch (p)
	{
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

uint64_t utcalc(struct timespec *ts)
{
	return (uint64_t) (ts->tv_sec * 1000000 + ts->tv_nsec / 1000); //us
}

int edge_detect
(
	struct avg *t,
	int *val, 
	uint64_t timer,
	struct arg *a
)
{
	int i;
	int valb = 0;
	int vala = 1;
	for (i=(0+t->fall)*a->edgew; i<(1+t->fall)*a->edgew; i++)
		valb = valb + val[i]; // or edgew 0's
	for (i=(1-t->fall)*a->edgew; i<(2-t->fall)*a->edgew; i++)
		vala = vala * val[i]; // and edgew 1's
	if ((valb == 0)&&(vala == 1))
	{
		printf("val[0]=%d", val[0]);
		avg_recw(t, timer, a);
		t->oldt = timer;
	}
	return 0;
}

void shift_sample(struct avg *t, struct arg *a)
{
	int i;
	for (i=(a->wlen-1); i>0; i--) t->sample[i] = t->sample[i-1]; 
}

int avg_recw(struct avg *t, uint64_t timer, struct arg *a)
{
	int i;
	if (t->oldt != 0)
	{
		shift_sample(t, a);
		t->sample[0] = timer - t->oldt;
		if (!a->debug)
			printf(", %s.sample[0]=%" PRIu64, t->label, t->sample[0]);
		t->iter++;
		t->avg = 0;
		t->wsize = (a->wlen-1 < t->iter) ? a->wlen : t->iter;
		for (i=0; i<a->wlen; i++)
		{
			t->avg = t->avg + t->sample[i] / t->wsize;
			if (a->debug)
				printf("\n%s.sample[%d]=%" PRIu64, 
					t->label, i, t->sample[i]);
		}
		printf(", %s.avg=%" PRIu64, t->label, t->avg);
		printf(", %s.wsize=%d\n", t->label, t->wsize);
	} else
		printf("\n");

	return 0;
}

int gpio_prep(struct pin *p, struct arg *a)
{
	char *scg = "/sys/class/gpio/";
	char *ofh = "Debug: Opening file";
	char *cfh = "Debug: Closing file";

	int pin = map_gpio(a->lpin);
	sprintf(p->pnum, "%d", pin);
	sprintf(p->Pnum, "%d", a->lpin);

	struct pin export;
	strcpy(export.fn, "/sys/class/gpio/export");
	if (a->debug) printf("%s %s\n", ofh, export.fn);
	export.fp = fopen(export.fn, "w");
	fprintf(export.fp, "%d", pin);
	fflush(export.fp);
	if (a->debug) printf("%s %s\n", cfh, export.fn);
	fclose(export.fp);

	struct pin direction;
	strcpy(direction.fn, scg);
	strcat(direction.fn, "D");
	strcat(direction.fn, p->Pnum);
	strcat(direction.fn, "/direction");
	if (a->debug) printf("%s %s\n", ofh, direction.fn);
	direction.fp = fopen(direction.fn, "w");
	fprintf(direction.fp, "in");
	fflush(direction.fp);
	if (a->debug) printf("%s %s\n", cfh, direction.fn);
	fclose(direction.fp);

	strcpy(p->fn, scg);
	strcat(p->fn, "D");
	strcat(p->fn, p->Pnum);
	strcat(p->fn, "/value");
	if (a->debug) printf("%s %s\n", ofh, p->fn);
	p->fp = fopen(p->fn, "r");
	
	return 0;
}

int gpio_cleanup(struct pin *p, struct arg *a)
{
	char *ofh = "Debug: Opening file";
	char *cfh = "Debug: Closing file";

	if (a->debug) printf("%s %s\n", cfh, p->fn);
	fclose(p->fp);
	
	struct pin unexport;
	strcpy(unexport.fn, "/sys/class/gpio/unexport");
	if (a->debug) printf("%s %s\n", ofh, unexport.fn);
	unexport.fp = fopen(unexport.fn, "w");
	
	fprintf(unexport.fp, p->pnum);
	fflush(unexport.fp);
	if (a->debug) printf("%s %s\n", cfh, unexport.fn);
	fclose(unexport.fp);

	return 0;
}	

int parse_opts(struct arg *a, int argc, char **argv)
{
	char c, *sleep=NULL, *wlength=NULL, *edgel=NULL, *listen=NULL;
	opterr = 0;
	while ((c = getopt(argc, argv, "dp:w:e:l:")) != -1)
	{
		switch (c)
		{
			case 'd':
				a->debug = 1;
				break;
			case 'p':
				sleep = optarg;
				break;
			case 'w':
				wlength = optarg;
				break;
			case 'e':
				edgel = optarg;
				break;
			case 'l':
				listen = optarg;
				break;
			case '?':
				fprintf(stderr, "Wat?\n");
				return 1;
			default: abort();
		}
	}
	a->poll = (sleep == NULL) ? 100000 : atoi(sleep);
	if (a->debug) printf("Debug: poll=%d\n", a->poll);
	a->wlen = (wlength == NULL) ? 5 : atoi(wlength);
	if (a->debug) printf("Debug: wlen=%d\n", a->wlen);
	a->edgew = (edgel == NULL) ? 1 : atoi(edgel);
	if (a->debug) printf("Debug: edgew=%d\n", a->edgew);
	a->lpin = (listen == NULL) ? 12 : atoi(listen);
	if (a->debug) printf("Debug: lpin=%d\n", a->lpin);

	return 0;
}
