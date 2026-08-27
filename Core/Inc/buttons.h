#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdint.h>

typedef enum
{
    BUTTON_GREEN = 0,
    BUTTON_BLUE1,
    BUTTON_BLUE2,
    BUTTON_YELLOW,
    BUTTON_RED
} ButtonId;

void Buttons_Init(void);
uint8_t Button_IsPressed(ButtonId button);

#endif
