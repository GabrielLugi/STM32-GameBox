#include "input.h"
#include "buttons.h"
#include "main.h"

#define INPUT_DEBOUNCE_MS       35U
#define INPUT_RELEASE_TIME_MS   40U

/*
 * INTERRUPT:
 * EXTI callback zapisuje kandidata i vrijeme prekida.
 */
static volatile InputEvent pendingEvent = INPUT_NONE;
static volatile uint32_t pendingTime = 0U;

/*
 * Događaj koji ostatak programa vidi tijekom jednog prolaza petlje.
 */
static InputEvent currentEvent = INPUT_NONE;

/*
 * Nakon prihvaćenog pritiska novi događaji su zaključani
 * sve dok se sve tipke ne otpuste.
 */
static uint8_t inputArmed = 1U;
static uint32_t allReleasedSince = 0U;

/* ------------------------------------------------------------------------- */

static uint8_t Input_AllButtonsReleased(void)
{
    return
        (Button_IsPressed(BUTTON_BLUE1) == 0U) &&
        (Button_IsPressed(BUTTON_BLUE2) == 0U) &&
        (Button_IsPressed(BUTTON_GREEN) == 0U) &&
        (Button_IsPressed(BUTTON_YELLOW) == 0U) &&
        (Button_IsPressed(BUTTON_RED) == 0U);
}

/* ------------------------------------------------------------------------- */

void Input_Init(void)
{
    pendingEvent = INPUT_NONE;
    pendingTime = 0U;

    currentEvent = INPUT_NONE;

    inputArmed = 1U;
    allReleasedSince = 0U;
}

/* ------------------------------------------------------------------------- */

void Input_EXTI_Callback(uint16_t gpioPin)
{
    InputEvent event = INPUT_NONE;

    /*
     * INTERRUPT:
     * Ovdje samo određujemo koja je tipka izazvala EXTI prekid.
     * Ne izvršavamo logiku menija ili igre unutar prekida.
     */

    if (gpioPin == _1PLAVA_A2_Pin)
    {
        event = INPUT_BLUE1;
    }
    else if (gpioPin == _2PLAVA_A1_Pin)
    {
        event = INPUT_BLUE2;
    }
    else if (gpioPin == ZELENA_A3_Pin)
    {
        event = INPUT_GREEN;
    }
    else if (gpioPin == ZUTA_D4_Pin)
    {
        event = INPUT_YELLOW;
    }
    else if (gpioPin == CRVENA_D2_Pin)
    {
        event = INPUT_RED;
    }

    /*
     * Ne prihvaćamo novi događaj dok prethodna tipka
     * nije potpuno otpuštena.
     */
    if ((event != INPUT_NONE) &&
        (inputArmed != 0U) &&
        (pendingEvent == INPUT_NONE))
    {
        pendingEvent = event;
        pendingTime = HAL_GetTick();
    }
}

/* ------------------------------------------------------------------------- */

void Input_Update(void)
{
    uint32_t now = HAL_GetTick();
    InputEvent candidate;

    /*
     * currentEvent vrijedi samo jedan prolaz glavne petlje.
     */
    currentEvent = INPUT_NONE;

    /*
     * Ako je ulaz zaključan, čekamo da se sve tipke otpuste.
     */
    if (inputArmed == 0U)
    {
        if (Input_AllButtonsReleased() != 0U)
        {
            if (allReleasedSince == 0U)
            {
                allReleasedSince = now;
            }
            else if ((now - allReleasedSince) >= INPUT_RELEASE_TIME_MS)
            {
                inputArmed = 1U;
                allReleasedSince = 0U;
            }
        }
        else
        {
            allReleasedSince = 0U;
        }

        return;
    }

    /*
     * Nema novog EXTI događaja.
     */
    if (pendingEvent == INPUT_NONE)
    {
        return;
    }

    /*
     * TIMER / DEBOUNCE:
     *
     * Čekamo da prođe debounce vrijeme od EXTI događaja.
     * Međutim, više NE zahtijevamo da tipka nakon 35 ms
     * još uvijek bude fizički pritisnuta.
     *
     * Sam EXTI rub već predstavlja registrirani pritisak.
     */
    if ((now - pendingTime) < INPUT_DEBOUNCE_MS)
    {
        return;
    }

    /*
     * Sigurno preuzimanje događaja koji zapisuje EXTI prekid.
     */
    __disable_irq();

    candidate = pendingEvent;
    pendingEvent = INPUT_NONE;

    __enable_irq();

    /*
     * INTERRUPT:
     *
     * EXTI događaj prihvaćamo kao jedan valjani pritisak.
     * Time će i vrlo kratak pritisak biti registriran.
     */
    if (candidate != INPUT_NONE)
    {
        currentEvent = candidate;

        /*
         * Zaključavamo novi događaj dok se sve tipke
         * ponovno ne otpuste.
         */
        inputArmed = 0U;
        allReleasedSince = 0U;
    }
}

/* ------------------------------------------------------------------------- */

InputEvent Input_GetEvent(void)
{
    return currentEvent;
}

/* ------------------------------------------------------------------------- */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    Input_EXTI_Callback(GPIO_Pin);
}
