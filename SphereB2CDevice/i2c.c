#include <errno.h>
#include <applibs/i2c.h>
#include <applibs/log.h>
#include <string.h>
#include "i2c.h"

int i2c_Init() {

	i2cFd = -1;
	i2cFd = I2CMaster_Open(SHARED_I2C);
	if (i2cFd < 0) {
		Log_Debug("ERROR: I2CMaster_Open: errno=%d (%s)\n", errno, strerror(errno));
		return -1;
	}

	int result = I2CMaster_SetBusSpeed(i2cFd, I2C_BUS_SPEED_STANDARD);
	if (result != 0) {
		Log_Debug("ERROR: I2CMaster_SetBusSpeed: errno=%d (%s)\n", errno, strerror(errno));
		return -1;
	}

	result = I2CMaster_SetTimeout(i2cFd, 100);
	if (result != 0) {
		Log_Debug("ERROR: I2CMaster_SetTimeout: errno=%d (%s)\n", errno, strerror(errno));
		return -1;
	}

	// This default address is used for POSIX read and write calls.  The AppLibs APIs take a target address argument for each read or write.
	result = I2CMaster_SetDefaultTargetAddress(i2cFd, NFC_ADDRESS);
	if (result != 0) {
		Log_Debug("ERROR: I2CMaster_SetDefaultTargetAddress: errno=%d (%s)\n", errno, strerror(errno));
		return -1;
	}

	return 0;
}