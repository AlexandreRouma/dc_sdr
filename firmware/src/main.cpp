#include <stdio.h>
#include "pico/stdlib.h"

#include "st77xx.h"
#include "tpl0401.h"

#include "parrots.h"

int main() {
    // Initialize LCD display
    ST77XX::Driver lcd(3, 2, 1, 4, -1, 0, 5);
    lcd.reset(ST77XX::RESET_HARDWARE);
    lcd.setSleep(false);
    lcd.setPartial(false);
    lcd.setIdle(false);
    lcd.setColorMode(ST77XX::COLOR_MODE_16BIT);

    // Draw test image to LCD
    lcd.blit(0, 0, 240, 240, parrots_bmp);

    // Unblank
    lcd.setBlank(false);

    while(1);
}
