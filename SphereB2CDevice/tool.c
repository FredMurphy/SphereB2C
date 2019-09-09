/*
*         Copyright (c), NXP Semiconductors Caen / France
*
*                     (C)NXP Semiconductors
*       All rights are reserved. Reproduction in whole or in part is
*      prohibited without the written consent of the copyright owner.
*  NXP reserves the right to make changes without notice at any time.
* NXP makes no warranty, expressed, implied or statutory, including but
* not limited to any implied warranty of merchantability or fitness for any
*particular purpose, or that the use will not infringe any third party patent,
* copyright or trademark. NXP must not be liable for any loss or damage
*                          arising from its use.
*/

#include <stdint.h>
#include <time.h>

void Sleep(unsigned int ms)
{
	struct timespec sleepTime = { 0, ms*1000000 };

	//for (unsigned int i=0; i<ms; i++)
	nanosleep(&sleepTime, NULL);
}
