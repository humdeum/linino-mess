#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <stdlib.h> // exit
#include <sys/stat.h>
#include <pthread.h>
#include <string.h>
#include <math.h>
#include <termios.h>
using namespace std;

//int Gargc;
//char **Gargv;
unsigned int freq=10;

class Gpio
{
private:
	unsigned int pin;
	string d;
	pthread_mutex_t mtx;
	bool debug;

public:
	Gpio(unsigned int p, int argc, char **argv) : pin(p)
	{
		if (argc == 2)
			if (strcmp(argv[1], "-d") == 0)
				debug = true;

		pthread_mutex_init(&mtx, NULL);
		
		// switch argv instead
		stringstream ss;
		ss << p - 100 - 2;
		d = ss.str();
		const char *gpioPtr = ("/sys/class/gpio/D" + d).c_str();
		struct stat sb;
		if (debug)
			cout << "Testing if D" + d + " is created" << endl;
		if (! (stat(gpioPtr, &sb) == 0 && S_ISDIR(sb.st_mode)) )
		{
			ofstream exportf;
			exportf.open("/sys/class/gpio/export");
			if (debug)
				cout << "Creating gpio D" << d << endl;
			exportf << p;
			if (debug)
				cout << "Closing handle" << endl;
			exportf.close();
		} else {
			if (debug)
				cout << "gpio is already created" << endl;
		}
	}
	void direction(string dir)
	{
		pthread_mutex_lock(&mtx);
		ofstream direction;
		direction.open( ("/sys/class/gpio/D" + d + "/direction").c_str() );
		direction << dir;
		direction.close();
		pthread_mutex_unlock(&mtx);
	}
	void value(unsigned int v)
	{
		pthread_mutex_lock(&mtx);
		ofstream value;
		value.open( ("/sys/class/gpio/D" + d + "/value").c_str() );
		if (debug)
			cout << v << endl;
		value << v;
		value.close();
		pthread_mutex_unlock(&mtx);
	}
}; //D13(115, Gargc, Gargv);

void *pwmSine(void *p)
{
	Gpio *pin = (Gpio*) p;
	int n = pow(2,6);
	int s[n];
	double dx = 4 * asin(1) / n;
	unsigned int i = 0;

	pin->direction("out");

	for (i=0; i<n; i++)
	{
		s[i] = round( pow(2, 9) * (1 + sin(i*dx)) );
	}

	i = 0;
	while(1)
	{
		pin->value(1);
		usleep(100/freq * (pow(2,10) - s[i % n]) );
		
		pin->value(0);
		usleep(100/freq * s[i % n]);

		i++;
	}
}

int getch(unsigned int *cPtr, termios *oldtPtr, termios *newtPtr)
{
	int e;
	tcsetattr(STDIN_FILENO, TCSANOW, newtPtr);
	e = (*cPtr = getchar());
	tcsetattr(STDIN_FILENO, TCSANOW, oldtPtr);
	return e;
}

void *readInput(void *vPtr)
{
	unsigned int c=0;
	static struct termios oldt, newt;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	cfmakeraw(&newt);
	while (getch(&c, &oldt, &newt) > 0)
	{
		//cout << c << endl;
		switch (c)
		{
			case 3: // ctrl+c
				exit(0);
			case '1': 
				cout << --freq << endl;
				break;
			case '2':
				cout << ++freq << endl;
				break;
			default :
				cout << "wat" << endl;
		}
		usleep(100);
	}
}

int main(int argc, char **argv)
{
//	Gargc = argc;
//	Gargv = argv;
	//Gpio D13(115, argc, argv);
	Gpio D10(106, argc, argv);

	pthread_t gpio_thread[3];

	//pthread_create(&gpio_thread[0], NULL, pwmSine, (void*) &D13 );
	pthread_create(&gpio_thread[1], NULL, pwmSine, (void*) &D10 );
	pthread_create(&gpio_thread[2], NULL, readInput, NULL);

	//pthread_join(gpio_thread[0], NULL);
	pthread_join(gpio_thread[1], NULL);
	pthread_join(gpio_thread[2], NULL);
}
