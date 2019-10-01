#include <stdint.h>
#include "display_settings.h"

#define OLED_TITLE_X      0
#define OLED_TITLE_Y      0 
#define OLED_LINE_1_X     0
#define OLED_LINE_1_Y     32
#define OLED_LINE_2_X     0
#define OLED_LINE_2_Y     44

#define FONT_SIZE_TITLE   3
#define FONT_SIZE_LINE    1

void init_display(void);
void clear_display(void);
void prompt_nfc(void);
void prompt_button(void);