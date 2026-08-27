#include "buzzer.h"
#include "main.h"
#include "tim.h"

static uint32_t buzzerStopTime = 0;
static uint8_t buzzerActive = 0;

void BUZZER_Init(void)
{
    HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_3);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, 0);
    buzzerActive = 0;
}

void BUZZER_Play(uint16_t frequency, uint16_t duration)
{
    if (frequency == 0U || duration == 0U)
    {
        BUZZER_Off();
        return;
    }

    uint32_t timerClock = 1000000U;
    uint32_t period = (timerClock / frequency) - 1U;

    HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_3);

    __HAL_TIM_SET_AUTORELOAD(&htim2, period);
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    HAL_TIM_GenerateEvent(&htim2, TIM_EVENTSOURCE_UPDATE);

    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, period / 2U);

    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);

    buzzerStopTime = HAL_GetTick() + duration;
    buzzerActive = 1U;
}

void BUZZER_Update(void)
{
    if (buzzerActive && HAL_GetTick() >= buzzerStopTime)
    {
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, 0);
        HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_3);
        buzzerActive = 0;
    }
}

void BUZZER_Beep(uint16_t ms)
{
    BUZZER_Play(2000, ms);
}

void BUZZER_On(void)
{
    BUZZER_Play(2000, 1000);
}

void BUZZER_Off(void)
{
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, 0);
    HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_3);
    buzzerActive = 0;
}
