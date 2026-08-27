#ifndef LCD_H
#define LCD_H

#include <stdint.h>
#include <stdio.h>

// colors
#define BLACK 0x0000
#define WHITE 0xFFFF
#define RED   0xF800
#define GREEN 0x07E0
#define BLUE  0x001F
#define CYAN  0x07FF
#define YELLOW 0xFFE0

void LCD_Init(void);
void LCD_Clear(uint16_t color);
void LCD_SetTextColor(uint16_t color);
void LCD_DisplayString(int x, int y, const char *text);

// low level (ILI9341 SPI driver wrapper)
void ILI9341_SendCommand(uint8_t cmd);
void ILI9341_SendData(uint8_t data);

void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void LCD_DrawChar(uint16_t x, uint16_t y, char c);
void LCD_DrawString(uint16_t x, uint16_t y, const char *text);
void LCD_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void LCD_DrawCharScaled(uint16_t x, uint16_t y, char c, uint8_t scale);
void LCD_DrawStringScaled(uint16_t x, uint16_t y, const char *text, uint8_t scale);
void LCD_DrawStringCentered(uint16_t y, const char *text, uint8_t scale);

void LCD_DrawLine(uint16_t x0, uint16_t y0,
                  uint16_t x1, uint16_t y1,
                  uint16_t color);

void LCD_DrawRect(uint16_t x,
                  uint16_t y,
                  uint16_t w,
                  uint16_t h,
                  uint16_t color);

void LCD_FillCircle(uint16_t x0,
                    uint16_t y0,
                    uint16_t radius,
                    uint16_t color);
#endif
