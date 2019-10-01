#include <stdint.h>
#include <time.h>

/*
Sleep for specified numbe rof milliseconds. Method conforms to signature expected by NXP code.
*/
void Sleep(unsigned int ms)
{
	struct timespec sleepTime = { 0, ms*1000000 };
	nanosleep(&sleepTime, NULL);
}
