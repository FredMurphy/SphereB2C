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
#include <applibs/i2c.h>
#include <applibs/log.h>
#include <errno.h>
#include <time.h>
#include "tool.h"
#include "tml.h"

//SemaphoreHandle_t IrqSem = NULL;
//i2c_master_transfer_t masterXfer;

static int i2cFd = -1;
static int resetFd = -1;
static int irqFd = -1;
static GPIO_Value_Type irqValue;

typedef enum {ERROR = 0, SUCCESS = !ERROR} Status;

//void PORTC_IRQHandler(void)
//{
//	//if (GPIO_ReadPinInput(NXPNCI_IRQ_GPIO, NXPNCI_IRQ_PIN) == 1)
//	//{
//	//    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
//	//	GPIO_ClearPinsInterruptFlags(NXPNCI_IRQ_GPIO, 1U << NXPNCI_IRQ_PIN);
//	//	xSemaphoreGiveFromISR(IrqSem, &xHigherPriorityTaskWoken);
//	//}
//}


static ssize_t I2C_WRITE(uint8_t *pBuff, uint16_t buffLen)
{
    //masterXfer.direction = kI2C_Write;
    //masterXfer.data = pBuff;
    //masterXfer.dataSize = buffLen;

    //return I2C_MasterTransferBlocking(NXPNCI_I2C_INSTANCE, &masterXfer);
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

	i2cFd = I2CMaster_Open(NFC_CLICK_I2C);
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

	// This default address is used for POSIX read and write calls.  The AppLibs APIs take a target
	// address argument for each read or write.
	result = I2CMaster_SetDefaultTargetAddress(i2cFd, NFC_ADDRESS);
	if (result != 0) {
		Log_Debug("ERROR: I2CMaster_SetDefaultTargetAddress: errno=%d (%s)\n", errno, strerror(errno));
		return -1;
	}

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
	
	//i2c_master_config_t masterConfig;
    //uint32_t sourceClock;

    //gpio_pin_config_t irq_config = {kGPIO_DigitalInput, 0,};
    //gpio_pin_config_t ven_config = {kGPIO_DigitalOutput, 0,};

    //GPIO_PinInit(NXPNCI_IRQ_GPIO, NXPNCI_IRQ_PIN, &irq_config);
    //GPIO_PinInit(NXPNCI_VEN_GPIO, NXPNCI_VEN_PIN, &ven_config);

    //I2C_MasterGetDefaultConfig(&masterConfig);
    //masterConfig.baudRate_Bps = NXPNCI_I2C_BAUDRATE;
    //sourceClock = CLOCK_GetFreq(I2C0_CLK_SRC);
    //masterXfer.slaveAddress = NXPNCI_I2C_ADDR;
    //masterXfer.subaddress = 0;
    //masterXfer.subaddressSize = 0;
    //masterXfer.flags = kI2C_TransferDefaultFlag;
    //I2C_MasterInit(NXPNCI_I2C_INSTANCE, &masterConfig, sourceClock);

    //IrqSem = xSemaphoreCreateBinary();

    return SUCCESS;
}

static Status tml_DeInit(void) {
	//vSemaphoreDelete(IrqSem);
	//GPIO_ClearPinsOutput(NXPNCI_VEN_GPIO, 1U << NXPNCI_VEN_PIN);
    return SUCCESS;
}


static Status tml_Reset(void) {

	GPIO_SetValue(resetFd, GPIO_Value_Low);
	Sleep(10);
	GPIO_SetValue(resetFd, GPIO_Value_High);
	
	return SUCCESS;
}

//static Status tml_Tx(uint8_t *pBuff, uint16_t buffLen) {
//    if (I2C_WRITE(pBuff, buffLen) < 0)
//    {
//		nanosleep(&sleep10ms, NULL);
//    	if(I2C_WRITE(pBuff, buffLen) < 0)
//    	{
//    		return ERROR;
//    	}
//    }
//
//	return SUCCESS;
//}
//
//static Status tml_Rx(uint8_t *pBuff, uint16_t buffLen, uint16_t *pBytesRead) {
//	ssize_t bytes =  I2C_READ(pBuff, buffLen);
//	*pBytesRead = bytes;
//
//	if (bytes < 0)
//		return ERROR;
//
//	return SUCCESS;
//	/*
//	if(I2C_READ(pBuff, 3) >= 0)
//    {
//    	if ((pBuff[2] + 3) <= buffLen)
//    	{
//			if (pBuff[2] > 0)
//			{
//				if(I2C_READ(&pBuff[3], pBuff[2]) >= 0)
//				{
//					*pBytesRead = pBuff[2] + 3;
//				}
//				else return ERROR;
//			} else
//			{
//				*pBytesRead = 3;
//			}
//    	}
//		else return ERROR;
//   }
//    else return ERROR;
//
//	return SUCCESS;
//	*/
//}

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

