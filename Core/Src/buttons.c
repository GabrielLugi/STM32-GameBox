#include "buttons.h"
#include "main.h"

void Buttons_Init(void)
{
}

uint8_t Button_IsPressed(ButtonId button)
{
    switch (button)
    {
        case BUTTON_BLUE1:
            return HAL_GPIO_ReadPin(_1PLAVA_A2_GPIO_Port,
                                    _1PLAVA_A2_Pin) == GPIO_PIN_RESET;

        case BUTTON_BLUE2:
            return HAL_GPIO_ReadPin(_2PLAVA_A1_GPIO_Port,
                                    _2PLAVA_A1_Pin) == GPIO_PIN_RESET;

        case BUTTON_GREEN:
            return HAL_GPIO_ReadPin(ZELENA_A3_GPIO_Port,
                                    ZELENA_A3_Pin) == GPIO_PIN_RESET;

        case BUTTON_YELLOW:
            return HAL_GPIO_ReadPin(ZUTA_D4_GPIO_Port,
                                    ZUTA_D4_Pin) == GPIO_PIN_RESET;

        case BUTTON_RED:
            return HAL_GPIO_ReadPin(CRVENA_D2_GPIO_Port,
                                    CRVENA_D2_Pin) == GPIO_PIN_RESET;

        default:
            return 0;
    }
}
