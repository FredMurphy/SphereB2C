/*
* Written to perform the same function as NXP implementation for their devices
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
static GPIO_Value_Type irqValue;

typedef enum {ERROR = 0, SUCCESS = !ERROR} Status;

static ssize_t I2C_WRITE(uint8_t *pBuff, uint16_t buffLen)
{
	ssize_t transferredBytes = I2CMaster_Write(i2cFd, NFC_ADDRESS, pBuff, buffLen);
	if (transferredBytes < 0) {
		Log_Debug("ERROR: I2CMaster_Write: errno=%d (%s)\n", errno, strerror(errno));
	}

	return transferredBytes;

}

static ssize_t I2C_READ(uint8_t *pBuff, uint16_t buffLen) {
	ssize_t transferredBytes = I2CMaster_Read(i2cFd, NFC_ADDRESS, pBuff, buffLen);
	return transferredBytes;
}

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


static Status tml_Reset(void) {

	GPIO_SetValue(resetFd, GPIO_Value_Low);
	Sleep(10);
	GPIO_SetValue(resetFd, GPIO_Value_High);
	
	return SUCCESS;
}

static Status tml_WaitForRx(uint32_t timeout) {

	GPIO_Value_Type irq = GPIO_Value_Low;
	if (timeout == 0) timeout--;

	while (irq == GPIO_Value_Low && timeout-- > 0) {
		GPIO_GetValue(irqFd, &irq);
		Sleep(1);
	}

	return (irq == GPIO_Value_Low ? ERROR : SUCCESS);
}

void tml_Connect(void) {
	tml_Init();
	tml_Reset();
}

void tml_Disconnect(void) {
	tml_DeInit();
}

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

void tml_SendThenReceive(uint8_t* pTxBuffer, uint16_t TxBufferLen, uint8_t* pRxBuffer, uint16_t RxBufferLen, uint16_t* pBytesReceived, uint16_t timeout) {

	ssize_t bytesReceived = I2CMaster_WriteThenRead(i2cFd, NFC_ADDRESS, pTxBuffer, TxBufferLen, pRxBuffer, RxBufferLen);
	if (bytesReceived < 0) {
		Log_Debug("ERROR: I2CMaster_WriteThenRead: errno=%d (%s)\n", errno, strerror(errno));
		bytesReceived = 0;
	}

	*pBytesReceived = bytesReceived;
}

