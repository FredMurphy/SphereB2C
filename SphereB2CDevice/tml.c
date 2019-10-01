/*
Written to perform the same function as NXP implementation for their devices
*/

#include <stdint.h>
#include <applibs/i2c.h>
#include <applibs/log.h>
#include <errno.h>
#include <time.h>
#include "tool.h"
#include "tml.h"
#include "i2c.h"

static int resetFd = -1;
static int irqFd = -1;

typedef enum {ERROR = 0, SUCCESS = !ERROR} Status;

/*
Initialize Click NFC
*/
static Status tml_Init(void) {

	resetFd = GPIO_OpenAsOutput(NFC_RST_PIN, GPIO_OutputMode_PushPull, GPIO_Value_High);
	if (resetFd < 0) {
		Log_Debug("Error opening GPIO: %s (%d). Check that app_manifest.json includes the GPIO used.\n", strerror(errno), errno);
		return -1;
	}

	irqFd = GPIO_OpenAsInput(NFC_IRQ_PIN);
	if (irqFd < 0) {
		Log_Debug("Error opening GPIO: %s (%d). Check that app_manifest.json includes the GPIO used.\n", strerror(errno), errno);
		return -1;
	}

    return SUCCESS;
}

static Status tml_DeInit(void) {
	// Not needed in this implementation
    return SUCCESS;
}

/*
Reset Click NFC device
*/
static Status tml_Reset(void) {

	GPIO_SetValue(resetFd, GPIO_Value_Low);
	Sleep(10);
	GPIO_SetValue(resetFd, GPIO_Value_High);
	
	return SUCCESS;
}

/*
Read I2C data from Click NFC (with timeout)
*/
static Status tml_WaitForRx(uint32_t timeout) {

	GPIO_Value_Type irq = GPIO_Value_Low;
	if (timeout == 0) timeout--;

	while (irq == GPIO_Value_Low && timeout-- > 0) {
		GPIO_GetValue(irqFd, &irq);
		Sleep(1);
	}

	return (irq == GPIO_Value_Low ? ERROR : SUCCESS);
}

/*
Connect to and reset Click NFC device
*/
void tml_Connect(void) {
	tml_Init();
	tml_Reset();
}

/*
Disconnect Click NFC
*/
void tml_Disconnect(void) {
	tml_DeInit();
}

/*
Send I2C data to Click NFC
*/
void tml_Send(uint8_t *pBuffer, uint16_t BufferLen, uint16_t *pBytesSent) {

	uint8_t retries = 10;
	ssize_t transferredBytes = 0;

	while (transferredBytes <= 0 && retries-- > 0) {
		transferredBytes = I2CMaster_Write(i2cFd, NFC_ADDRESS, pBuffer, BufferLen);
		if (transferredBytes < 0) {
			Log_Debug("ERROR: I2CMaster_Write: errno=%d (%s)\n", errno, strerror(errno));
			transferredBytes = 0;
			Sleep(10);
		}

	}
	*pBytesSent = transferredBytes;
}

/*
Receive I2C data from Click NFC
*/
void tml_Receive(uint8_t *pBuffer, uint16_t BufferLen, uint16_t *pBytes, uint16_t timeout) {

	if (tml_WaitForRx(timeout) == ERROR)
		*pBytes = 0;
	else {
		uint8_t retries = 10;
		ssize_t bytesRead = 0;
		while (bytesRead <= 0 && retries-- > 0) {
			bytesRead = I2CMaster_Read(i2cFd, NFC_ADDRESS, pBuffer, BufferLen);
			if (bytesRead < 0) {
				Log_Debug("ERROR: I2CMaster_Read: errno=%d (%s)\n", errno, strerror(errno));
				bytesRead = 0;
				Sleep(10);
			}
		}

		// Correct duplicated first byte
		uint8_t reply0 = (pBuffer[1] & 0xF0);
		if (bytesRead > 1 && ((reply0 == 0x40) || (reply0 == 0x60))) {
			//Log_Debug("INFO: tml_Receive removing duplicate start byte\n");
			for (ssize_t i = 0; i < bytesRead-2; i++) {
				pBuffer[i] = pBuffer[i+1];
			}
		}
		*pBytes = bytesRead;
	}
}
