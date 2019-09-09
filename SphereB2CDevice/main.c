#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#include <applibs/log.h>
#include <applibs/gpio.h>

#include "mt3620_avnet_dev.h"
#include "azure_iot_utilities.h"
#include "Nfc.h"

static volatile int terminationRequired = false;

static int redLedFd;
static int greenLedFd;
static int blueLedFd;

static int buttonAFd;
static int buttonBFd;
static GPIO_Value_Type buttonAState;
static GPIO_Value_Type buttonBState;

int configureGpio(void);
const struct timespec pollTimespec = { 1, 0 };

char message[50] = "X";

/*
Incoming direct method call from Azure IoT Hub
*/
int directMethodCall(const char* directMethodName, const char* payload, size_t payloadSize, char** responsePayload, size_t* responsePayloadSize) {

	strcpy(message, "{\"error\":true,\"method\":\"none\", \"value\":\"timeout\"}");

	if (strcmp(directMethodName, "Authenticate")) {
		return 404;
	}

	GPIO_SetValue(blueLedFd, GPIO_Value_Low);

	for (int i = 5; i > 0; i--) {
		GPIO_GetValue(buttonAFd, &buttonAState);
		if (buttonAState == GPIO_Value_Low) {
			strcpy(message, "{\"method\":\"button\", \"value\":\"button_a\"}");
			break;
		}
		GPIO_GetValue(buttonBFd, &buttonBState);
		if (buttonBState == GPIO_Value_Low) {
			//strcpy(message, "{\"method\":\"button\", \"value\":\"button_b\"}");
			strcpy(message, "{\"method\":\"nfc\", \"value\":\"1234\"}");
			break;
		}

		AzureIoT_DoPeriodicTasks();
		nanosleep(&pollTimespec, NULL);
	}
	Log_Debug(message);
	
	GPIO_SetValue(blueLedFd, GPIO_Value_High);

	*responsePayload = message;
	*responsePayloadSize = strlen(message);
	
	return 200;
}

int main(void)
{
	if (configureGpio() < 0)
		return -1;

	if (!AzureIoT_SetupClient()) {
		Log_Debug("ERROR: Failed to set up IoT Hub client\n");
		return -1;
	}
	AzureIoT_SetDirectMethodCallback(&directMethodCall);

	if (InitNfc() < 0) {
		Log_Debug("ERROR: Failed initialize NFC reader\n");
		return -1;
	}

    const struct timespec sleepTime = {1, 0};

	char tag[20];

	if (GetNfcTagId(tag, 10000) == 0) {
		Log_Debug("NFC tag: %s\n", tag);
	}
	else {
		Log_Debug("NFC tag timeout\n");
	}

    while (!terminationRequired) {

		if (GPIO_GetValue(buttonAFd, &buttonAState) < 0) {
			Log_Debug("Error reading GPIO: %s (%d).\n", strerror(errno), errno);
			terminationRequired = true;
		}
			
		if (GPIO_SetValue(redLedFd, buttonAState) < 0) {
			Log_Debug("Error setting RED LED: %s (%d).\n", strerror(errno), errno);
			terminationRequired = true;
		}

        GPIO_SetValue(greenLedFd, GPIO_Value_Low);
        nanosleep(&sleepTime, NULL);
        GPIO_SetValue(greenLedFd, GPIO_Value_High);
        nanosleep(&sleepTime, NULL);

		// AzureIoT_DoPeriodicTasks() needs to be called frequently in order to keep active the flow of data with the Azure IoT Hub
		AzureIoT_DoPeriodicTasks();

    }

	Log_Debug("Exiting");

}


int configureGpio() {

	Log_Debug("Configuring GPIO");

	redLedFd = GPIO_OpenAsOutput(MT3620_RDB_LED1_RED, GPIO_OutputMode_PushPull, GPIO_Value_High);
	if (redLedFd < 0) {
		Log_Debug("Error opening GPIO: %s (%d). Check that app_manifest.json includes the GPIO used.\n", strerror(errno), errno);
		return -1;
	}

	greenLedFd = GPIO_OpenAsOutput(MT3620_RDB_LED1_GREEN, GPIO_OutputMode_PushPull, GPIO_Value_High);
	if (greenLedFd < 0) {
		Log_Debug("Error opening GPIO: %s (%d). Check that app_manifest.json includes the GPIO used.\n", strerror(errno), errno);
		return -1;
	}

	blueLedFd = GPIO_OpenAsOutput(MT3620_RDB_LED1_BLUE, GPIO_OutputMode_PushPull, GPIO_Value_High);
	if (blueLedFd < 0) {
		Log_Debug("Error opening GPIO: %s (%d). Check that app_manifest.json includes the GPIO used.\n", strerror(errno), errno);
		return -1;
	}

	buttonAFd = GPIO_OpenAsInput(12);
	if (buttonAFd < 0) {
		Log_Debug("Error opening GPIO: %s (%d). Check that app_manifest.json includes the GPIO used.\n", strerror(errno), errno);
		return -1;
	}

	buttonBFd = GPIO_OpenAsInput(13);
	if (buttonBFd < 0) {
		Log_Debug("Error opening GPIO: %s (%d). Check that app_manifest.json includes the GPIO used.\n", strerror(errno), errno);
		return -1;
	}


	return 0;

}