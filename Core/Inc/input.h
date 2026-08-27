#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>

typedef enum
{
    INPUT_NONE = 0,

    INPUT_BLUE1,
    INPUT_BLUE2,
    INPUT_GREEN,
    INPUT_YELLOW,
    INPUT_RED

} InputEvent;

void Input_Init(void);

/*
 * Preuzima događaj koji je zabilježen u EXTI prekidu.
 * Poziva se iz glavne petlje.
 */
void Input_Update(void);

/*
 * Vraća događaj za trenutačni prolaz glavne petlje.
 */
InputEvent Input_GetEvent(void);

/*
 * Prima GPIO pin iz HAL_GPIO_EXTI_Callback funkcije.
 */
void Input_EXTI_Callback(uint16_t gpioPin);

#endif
