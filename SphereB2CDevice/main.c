#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#include <applibs/log.h>
#include <applibs/gpio.h>
#include <stdlib.h>

#include "mt3620_avnet_dev.h"
#include "azure_iot_utilities.h"
#include "Nfc.h"
#include "display.h"

static volatile int terminationRequired = false;

static int redLedFd;
static int greenLedFd;
static int blueLedFd;

static int buttonAFd;
static int buttonBFd;
static GPIO_Value_Type buttonAState;
static GPIO_Value_Type buttonBState;

int configureGpio(void);
const struct timespec buttonPollTimespec = { 0, 100000000 }; // 100ms


/*
Incoming direct method call from Azure IoT Hub
Typical payloads:
{
	"deviceName": "MyDeviceName",
	"secondaryMethod": "nfc"
*/
char method[20] = { 0 };


char* parseSecondaryMethod(const char* payload, size_t payloadSize) {

	size_t nullTerminatedJsonSize = payloadSize + 1;
	char* nullTerminatedJsonString = (char*)malloc(nullTerminatedJsonSize);
	if (nullTerminatedJsonString == NULL) {
		Log_Debug("ERROR: Could not allocate buffer for twin update payload.\n");
		return method;
	}

	// Copy the provided buffer to a null terminated buffer.
	memcpy(nullTerminatedJsonString, payload, payloadSize);
	// Add the null terminator at the end.
	nullTerminatedJsonString[nullTerminatedJsonSize - 1] = 0;

	JSON_Value* rootProperties = NULL;
	rootProperties = json_parse_string(nullTerminatedJsonString);
	if (rootProperties == NULL) {
		Log_Debug("WARNING: Cannot parse the string as JSON content.\n");
		goto cleanup;
	}

	JSON_Object* rootObject = json_value_get_object(rootProperties);
	const char* parsedMethod = json_object_get_string(rootObject, "secondaryMethod");
	strcpy(method, parsedMethod);

cleanup:
	// Release the allocated memory.
	json_value_free(rootProperties);
	free(nullTerminatedJsonString);

	return method;
}

char message[100] = { 0 };
int directMethodCall(const char* directMethodName, const char* payload, size_t payloadSize, char** responsePayload, size_t* responsePayloadSize) {


	if (strcmp(directMethodName, "Authenticate")) {
		return 404;
	}

	JSON_Value* root_value = json_value_init_object();
	JSON_Object* root_object = json_value_get_object(root_value);

	GPIO_SetValue(blueLedFd, GPIO_Value_Low);

	char* requestedMethod = parseSecondaryMethod(payload, payloadSize);
	Log_Debug("secondaryMethod: %s\n", requestedMethod);
	json_object_set_string(root_object, "method", requestedMethod);
	json_object_set_string(root_object, "value", "not recognised");

	// secondary authentication is NFC tag
	if (strcmp("nfc", requestedMethod) == 0) {
		prompt_nfc();
		char tag[20];
		if (GetNfcTagId(tag, 10000) == 0) {
			Log_Debug("NFC tag: %s\n", tag);
			json_object_set_string(root_object, "value", tag);
		}
		else {
			Log_Debug("NFC tag timeout\n");
			json_object_set_string(root_object, "value", "timeout");
		}
	}
	// secondary authentication is button press
	else if (strcmp("button", requestedMethod) == 0) {
		prompt_button();
		json_object_set_string(root_object, "value", "timeout");
		// Check for button press (every 100ms for 10s)
		for (int i = 100; i > 0; i--) {
			GPIO_GetValue(buttonAFd, &buttonAState);
			if (buttonAState == GPIO_Value_Low) {
				json_object_set_string(root_object, "value", "button_a");
				break;
			}
			GPIO_GetValue(buttonBFd, &buttonBState);
			if (buttonBState == GPIO_Value_Low) {
				json_object_set_string(root_object, "value", "button_b");
				break;
			}

			AzureIoT_DoPeriodicTasks();
			nanosleep(&buttonPollTimespec, NULL);
		}
	}
	
	GPIO_SetValue(blueLedFd, GPIO_Value_High);
	clear_display();

	char* json = json_serialize_to_string(root_value);
	strcpy(message, json);
	// release allocated memory
	json_free_serialized_string(json);
	json_value_free(root_value);

	Log_Debug(message);
	*responsePayload = message;
	*responsePayloadSize = strlen(message);
	
	return 200;
}

int main(void)
{
	if (configureGpio() < 0)
		return -1;

	if (i2c_Init() < 0) {
		Log_Debug("Error initializing I2C.\n", strerror(errno), errno);
		return -1;
	}

	if (init_display() < 0)
		return -1;

	clear_display();

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