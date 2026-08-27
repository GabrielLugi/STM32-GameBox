#ifndef WS2812_H
#define WS2812_H

#include <stdint.h>

#define WS2812_LED_COUNT 8U

void WS2812_Init(void);
void WS2812_Clear(void);

void WS2812_SetPixel(uint8_t index,
                     uint8_t red,
                     uint8_t green,
                     uint8_t blue);

void WS2812_Fill(uint8_t red,
                 uint8_t green,
                 uint8_t blue);

void WS2812_Show(void);
uint8_t WS2812_IsBusy(void);

#endif
