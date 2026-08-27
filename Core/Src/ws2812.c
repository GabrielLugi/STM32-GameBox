#include "ws2812.h"
#include "main.h"
#include "tim.h"

/*
 * WS2812 hardverska konfiguracija:
 *
 * TIMER:
 * TIM3_CH1, izlaz PA6 / D12
 *
 * DMA:
 * DMA1 Stream 4, Channel 5
 *
 * TRIGGER:
 * TIM3 Capture/Compare 1 događaj pokreće DMA prijenos
 * sljedeće vrijednosti u TIM3->CCR1.
 */

#define WS2812_BITS_PER_LED    24U
#define WS2812_RESET_SLOTS     240U

#define WS2812_BUFFER_SIZE \
    ((WS2812_LED_COUNT * WS2812_BITS_PER_LED) + WS2812_RESET_SLOTS)

/*
 * Vrijednosti odgovaraju trenutačno funkcionalnoj konfiguraciji:
 *
 * TIM3 clock = 84 MHz
 * ARR = 109
 *
 * CCR = 32 -> logička nula
 * CCR = 64 -> logička jedinica
 */
#define WS2812_BIT_0           32U
#define WS2812_BIT_1           64U

/*
 * WS2812 očekuje redoslijed boja GRB.
 */
#define COLOR_GREEN            0U
#define COLOR_RED              1U
#define COLOR_BLUE             2U

static uint16_t dmaBuffer[WS2812_BUFFER_SIZE];
static uint8_t ledData[WS2812_LED_COUNT][3];

/* ------------------------------------------------------------------------- */

static void WS2812_WaitForPreviousTransfer(void)
{
    /*
     * DMA radi u NORMAL načinu.
     * Nakon završetka prijenosa hardver automatski briše EN bit.
     */
    while ((DMA1_Stream4->CR & DMA_SxCR_EN) != 0U)
    {
    }
}

/* ------------------------------------------------------------------------- */

static void WS2812_ClearDmaFlags(void)
{
    DMA1->HIFCR =
        DMA_HIFCR_CFEIF4  |
        DMA_HIFCR_CDMEIF4 |
        DMA_HIFCR_CTEIF4  |
        DMA_HIFCR_CHTIF4  |
        DMA_HIFCR_CTCIF4;
}

/* ------------------------------------------------------------------------- */

static uint32_t WS2812_EncodeByte(uint32_t index, uint8_t value)
{
    for (int8_t bit = 7; bit >= 0; bit--)
    {
        if ((value & (1U << bit)) != 0U)
        {
            dmaBuffer[index++] = WS2812_BIT_1;
        }
        else
        {
            dmaBuffer[index++] = WS2812_BIT_0;
        }
    }

    return index;
}

/* ------------------------------------------------------------------------- */

static void WS2812_BuildDmaBuffer(void)
{
    uint32_t index = 0U;

    for (uint32_t led = 0U; led < WS2812_LED_COUNT; led++)
    {
        /*
         * WS2812 redoslijed:
         * GREEN, RED, BLUE
         */
        index = WS2812_EncodeByte(
            index,
            ledData[led][COLOR_GREEN]);

        index = WS2812_EncodeByte(
            index,
            ledData[led][COLOR_RED]);

        index = WS2812_EncodeByte(
            index,
            ledData[led][COLOR_BLUE]);
    }

    /*
     * RESET / LATCH:
     * CCR vrijednost 0 drži podatkovnu liniju u LOW stanju.
     */
    while (index < WS2812_BUFFER_SIZE)
    {
        dmaBuffer[index++] = 0U;
    }
}

/* ------------------------------------------------------------------------- */

void WS2812_Init(void)
{
    WS2812_Clear();

    /*
     * TIMER:
     * CCR1 preload osigurava promjenu duty cyclea na granici
     * PWM perioda, a ne usred WS2812 bita.
     */
    TIM3->CCMR1 |= TIM_CCMR1_OC1PE;

    /*
     * Početno stanje podatkovne linije je LOW.
     */
    TIM3->CR1 &= ~TIM_CR1_CEN;
    TIM3->DIER &= ~TIM_DIER_CC1DE;
    TIM3->CCR1 = 0U;
    TIM3->CNT = 0U;

    WS2812_Show();
}

/* ------------------------------------------------------------------------- */

void WS2812_Clear(void)
{
    for (uint32_t led = 0U; led < WS2812_LED_COUNT; led++)
    {
        ledData[led][COLOR_GREEN] = 0U;
        ledData[led][COLOR_RED] = 0U;
        ledData[led][COLOR_BLUE] = 0U;
    }
}

/* ------------------------------------------------------------------------- */

void WS2812_SetPixel(uint8_t index,
                     uint8_t red,
                     uint8_t green,
                     uint8_t blue)
{
    if (index >= WS2812_LED_COUNT)
    {
        return;
    }

    ledData[index][COLOR_GREEN] = green;
    ledData[index][COLOR_RED] = red;
    ledData[index][COLOR_BLUE] = blue;
}

/* ------------------------------------------------------------------------- */

void WS2812_Fill(uint8_t red,
                 uint8_t green,
                 uint8_t blue)
{
    for (uint8_t led = 0U; led < WS2812_LED_COUNT; led++)
    {
        WS2812_SetPixel(led, red, green, blue);
    }
}

/* ------------------------------------------------------------------------- */

uint8_t WS2812_IsBusy(void)
{
    return ((DMA1_Stream4->CR & DMA_SxCR_EN) != 0U) ? 1U : 0U;
}

/* ------------------------------------------------------------------------- */

void WS2812_Show(void)
{
    WS2812_WaitForPreviousTransfer();
    WS2812_BuildDmaBuffer();

    /*
     * Zaustavi TIMER prije pripreme novog prijenosa.
     */
    TIM3->CR1 &= ~TIM_CR1_CEN;

    /*
     * Privremeno isključi TIM3 CC1 DMA request.
     */
    TIM3->DIER &= ~TIM_DIER_CC1DE;

    /*
     * Za svaki slučaj ugasi DMA Stream 4.
     */
    DMA1_Stream4->CR &= ~DMA_SxCR_EN;

    while ((DMA1_Stream4->CR & DMA_SxCR_EN) != 0U)
    {
    }

    WS2812_ClearDmaFlags();

    /*
     * DMA izvor:
     * kodirani WS2812 podaci u RAM-u.
     */
    DMA1_Stream4->M0AR = (uint32_t)dmaBuffer;

    /*
     * DMA odredište:
     * Capture/Compare registar TIM3 kanala 1.
     */
    DMA1_Stream4->PAR = (uint32_t)&TIM3->CCR1;

    DMA1_Stream4->NDTR = WS2812_BUFFER_SIZE;

    /*
     * TIMER počinje od nule i s LOW izlazom.
     */
    TIM3->CNT = 0U;
    TIM3->CCR1 = 0U;

    /*
     * Prvo uključujemo DMA.
     */
    DMA1_Stream4->CR |= DMA_SxCR_EN;

    /*
     * TRIGGER:
     * TIM3 CC1 događaj sada smije pokrenuti DMA prijenos.
     */
    TIM3->DIER |= TIM_DIER_CC1DE;

    /*
     * Omogući PWM izlaz TIM3_CH1.
     */
    TIM3->CCER |= TIM_CCER_CC1E;

    /*
     * Pokreni TIMER.
     */
    TIM3->CR1 |= TIM_CR1_CEN;
}
