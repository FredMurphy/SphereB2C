#include "display.h"

uint8_t init_display() {
	return sd1306_init();
}

void clear_display() {
	clear_oled_buffer();
	sd1306_refresh();
}

void prompt_nfc(void) {
	clear_oled_buffer();
	sd1306_draw_string(OLED_TITLE_X, OLED_TITLE_Y, "NFC", FONT_SIZE_TITLE, white_pixel);
	sd1306_draw_string(OLED_LINE_1_X, OLED_LINE_1_Y, "Authenticate by", FONT_SIZE_LINE, white_pixel);
	sd1306_draw_string(OLED_LINE_2_X, OLED_LINE_2_Y, "scanning NFC tag", FONT_SIZE_LINE, white_pixel);
	sd1306_refresh();
}

void prompt_button(void) {
	clear_oled_buffer();
	sd1306_draw_string(OLED_TITLE_X, OLED_TITLE_Y, "Button", FONT_SIZE_TITLE, white_pixel);
	sd1306_draw_string(OLED_LINE_1_X, OLED_LINE_1_Y, "Authenticate by", FONT_SIZE_LINE, white_pixel);
	sd1306_draw_string(OLED_LINE_2_X, OLED_LINE_2_Y, "pressing a button", FONT_SIZE_LINE, white_pixel);
	sd1306_refresh();

}