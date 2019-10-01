
#include "display.h"
#include "mt3620_avnet_dev.h"
#include "applibs_versions.h"
#include <errno.h>
#include <string.h>
#include <time.h>
#include <applibs/log.h>
#include <applibs/spi.h>
#include <applibs/gpio.h>
#include <soc/mt3620_spis.h>

#include "oledb_ssd1306.h"
#include "Avnet_GFX.h"

static struct timespec timeval;
#define delay(x) {timeval.tv_sec=0; timeval.tv_nsec=(x*1000000); nanosleep(&timeval,NULL);}

static int spiFd = -1;
static GPIO_Value_Type dcPin;      //OLED Display Command/Data signal
static GPIO_Value_Type rstPin;     //OLED Display Reset Pin

static int dcPinFd = -1;
static int rstPinFd = -1;

#define DISP_NORMAL     0
#define DISP_REVERSED   1
OLEDB_SSD1306* oled_display;

//
//SPI handler for OLED-B display
//
void spi_init(void)
{
	dcPinFd = GPIO_OpenAsOutput(MT3620_GPIO1, dcPin, GPIO_Value_High);
	rstPinFd = GPIO_OpenAsOutput(MT3620_GPIO17, rstPin, GPIO_Value_High);

	SPIMaster_Config config;
	int ret = SPIMaster_InitConfig(&config);
	if (ret != 0) {
		Log_Debug("ERROR: SPIMaster_InitConfig = %d errno = %s (%d)\n", ret, strerror(errno),
			errno);
		return -1;
	}
	config.csPolarity = SPI_ChipSelectPolarity_ActiveLow;
	spiFd = SPIMaster_Open(MT3620_SPI_ISU1, MT3620_SPI_CHIP_SELECT_B, &config);
	if (spiFd < 0) {
		Log_Debug("ERROR: SPIMaster_Open: errno=%d (%s)\n", errno, strerror(errno));
		return -1;
	}

	int result = SPIMaster_SetBusSpeed(spiFd, 400000);
	if (result != 0) {
		Log_Debug("ERROR: SPIMaster_SetBusSpeed: errno=%d (%s)\n", errno, strerror(errno));
		return -1;
	}

	result = SPIMaster_SetMode(spiFd, SPI_Mode_3);
	if (result != 0) {
		Log_Debug("ERROR: SPIMaster_SetMode: errno=%d (%s)\n", errno, strerror(errno));
		return -1;
	}
}

int spi_write(uint16_t cmd, uint8_t* b, int siz)
{
	SPIMaster_Transfer transfer;
	ssize_t            transferredBytes;
	const size_t       transferCount = siz;

	if (SPIMaster_InitTransfers(&transfer, 1) != 0)
		return -1;

	transfer.flags = SPI_TransferFlags_Write;
	transfer.writeData = b;
	transfer.length = siz;

	if (cmd == SSD1306_COMMAND) //if sending a Command
		GPIO_SetValue(dcPinFd, GPIO_Value_Low);

	transferredBytes = SPIMaster_TransferSequential(spiFd, &transfer, 1);

	if (cmd == SSD1306_COMMAND)
		GPIO_SetValue(dcPinFd, GPIO_Value_High);

	return (transferredBytes == transfer.length);
}

int oled_reset(void)
{
	GPIO_SetValue(rstPinFd, GPIO_Value_Low);
	delay(10);   //10 msec
	GPIO_SetValue(rstPinFd, GPIO_Value_High);
	delay(10);   //10 msec
	return 0;
}

void init_display() {

	oled_display = open_oled(spi_init, oled_reset, spi_write);
}

void clear_display() {

	oledb_clrDispBuff(oled_display);
	oledb_display(oled_display, DISP_NORMAL);
}

void prompt(char* title, char* text) {

	oledb_clrDispBuff(oled_display);

	AvnetGFX_setTextColor(1, 1);
	AvnetGFX_setCursor(0, 0);
	AvnetGFX_setTextSize(2);
	AvnetGFX_printText(title);

	AvnetGFX_setCursor(0, 20);
	AvnetGFX_setTextSize(1);
	AvnetGFX_printText(text);

	oledb_display(oled_display, DISP_REVERSED);

}
void prompt_nfc(void) {
	prompt("NFC", "Please scan tag");
}

void prompt_button(void) {
	prompt("Button", "Press a button");
}

