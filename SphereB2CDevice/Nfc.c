#include "Nfc.h"
#include <stdio.h>
#include <string.h>
#include <applibs/log.h>

unsigned char mode = NXPNCI_MODE_RW;
unsigned char DiscoveryTechnologies[] = {
	MODE_POLL | TECH_PASSIVE_NFCA
	//MODE_POLL | TECH_PASSIVE_NFCF,
	//MODE_POLL | TECH_PASSIVE_NFCB,
	//MODE_POLL | TECH_PASSIVE_15693 
};

int InitNfc() {

	/* Open connection to NXPNCI device */
	if (NxpNci_Connect() == NFC_ERROR) {
		Log_Debug("NxpNci_Connect: cannot connect to NXPNCI device\n");
		return -1;
	}
	Log_Debug("NxpNci_Connect: Success!\n");

	if (NxpNci_ConfigureSettings() == NFC_ERROR) {
		Log_Debug("Error: cannot configure NXPNCI settings\n");
		return -1;
	}


	if (NxpNci_ConfigureMode(mode) == NFC_ERROR)
	{
		Log_Debug("Error: cannot configure NXPNCI\n");
		return -1;
	}

	return 0;
}

int GetNfcTagId(char* tagBuffer, uint16_t timeout_ms) {

	NxpNci_RfIntf_t RfInterface;
	tagBuffer[0] = 0;

	if (NxpNci_StartDiscovery(DiscoveryTechnologies, sizeof(DiscoveryTechnologies)) != NFC_SUCCESS)
	{
		Log_Debug("Error: cannot start discovery\n");
		return -1;
	}

	Log_Debug("\nWAITING FOR DEVICE DISCOVERY\n");

	/* Wait until a peer is discovered */
	bool result = NxpNci_WaitForDiscoveryNotificationTimeout(&RfInterface, timeout_ms);
	NxpNci_StopDiscovery();

	if (result != NFC_SUCCESS) {
		Log_Debug("No NFC tag found\n");
		return -1;
	}

	if ((RfInterface.ModeTech & !MODE_MASK) == TECH_PASSIVE_NFCA) {
		for (int i = 0; i < RfInterface.Info.NFC_APP.NfcIdLen; i++) {
			sprintf(tagBuffer + strlen(tagBuffer), "%02X", RfInterface.Info.NFC_APP.NfcId[i]);
		}
		Log_Debug("NFC tag A: %s", tagBuffer);
	}
	Log_Debug("\n");
	return 0;
}

