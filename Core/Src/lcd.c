
#include "lcd.h"
#include "main.h"
#include "spi.h"
#include "font5x7.h"
#include <string.h>

#define LCD_WIDTH   240
#define LCD_HEIGHT  320

static uint16_t currentColor = WHITE;

static void LCD_Select(void)
{
    HAL_GPIO_WritePin(CS_TFT__D10_GPIO_Port, CS_TFT__D10_Pin, GPIO_PIN_RESET);
}

static void LCD_Unselect(void)
{
    HAL_GPIO_WritePin(CS_TFT__D10_GPIO_Port, CS_TFT__D10_Pin, GPIO_PIN_SET);
}

static void LCD_DC_Command(void)
{
    HAL_GPIO_WritePin(DC___D9_GPIO_Port, DC___D9_Pin, GPIO_PIN_RESET);
}

static void LCD_DC_Data(void)
{
    HAL_GPIO_WritePin(DC___D9_GPIO_Port, DC___D9_Pin, GPIO_PIN_SET);
}

static void LCD_Reset(void)
{
    HAL_GPIO_WritePin(RST___D8_GPIO_Port, RST___D8_Pin, GPIO_PIN_RESET);
    HAL_Delay(50);
    HAL_GPIO_WritePin(RST___D8_GPIO_Port, RST___D8_Pin, GPIO_PIN_SET);
    HAL_Delay(120);
}

void ILI9341_SendCommand(uint8_t cmd)
{
    LCD_Select();
    LCD_DC_Command();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    LCD_Unselect();
}

void ILI9341_SendData(uint8_t data)
{
    LCD_Select();
    LCD_DC_Data();
    HAL_SPI_Transmit(&hspi1, &data, 1, HAL_MAX_DELAY);
    LCD_Unselect();
}

static void ILI9341_SendDataBuffer(uint8_t *data, uint16_t size)
{
    LCD_Select();
    LCD_DC_Data();
    HAL_SPI_Transmit(&hspi1, data, size, HAL_MAX_DELAY);
    LCD_Unselect();
}

static void LCD_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    uint8_t data[4];

    ILI9341_SendCommand(0x2A);
    data[0] = x0 >> 8;
    data[1] = x0 & 0xFF;
    data[2] = x1 >> 8;
    data[3] = x1 & 0xFF;
    ILI9341_SendDataBuffer(data, 4);

    ILI9341_SendCommand(0x2B);
    data[0] = y0 >> 8;
    data[1] = y0 & 0xFF;
    data[2] = y1 >> 8;
    data[3] = y1 & 0xFF;
    ILI9341_SendDataBuffer(data, 4);

    ILI9341_SendCommand(0x2C);
}

void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT)
        return;

    LCD_SetAddressWindow(x, y, x, y);

    uint8_t data[2];

    data[0] = color >> 8;
    data[1] = color & 0xFF;

    LCD_Select();
    LCD_DC_Data();

    HAL_SPI_Transmit(&hspi1, data, 2, HAL_MAX_DELAY);

    LCD_Unselect();
}


void LCD_Init(void)
{
    HAL_GPIO_WritePin(BL___D5_GPIO_Port, BL___D5_Pin, GPIO_PIN_SET);

    LCD_Unselect();
    LCD_Reset();

    ILI9341_SendCommand(0x01);
    HAL_Delay(100);

    ILI9341_SendCommand(0x28);

    ILI9341_SendCommand(0x3A);
    ILI9341_SendData(0x55);

    ILI9341_SendCommand(0x36);
    ILI9341_SendData(0x48);

    ILI9341_SendCommand(0x11);
    HAL_Delay(120);

    ILI9341_SendCommand(0x29);
    HAL_Delay(20);

    LCD_Clear(BLACK);
}

void LCD_Clear(uint16_t color)
{
    LCD_SetAddressWindow(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);

    uint8_t data[2];
    data[0] = color >> 8;
    data[1] = color & 0xFF;

    LCD_Select();
    LCD_DC_Data();

    for (uint32_t i = 0; i < LCD_WIDTH * LCD_HEIGHT; i++)
    {
        HAL_SPI_Transmit(&hspi1, data, 2, HAL_MAX_DELAY);
    }

    LCD_Unselect();
}

void LCD_SetTextColor(uint16_t color)
{
    currentColor = color;
}

void LCD_DrawChar(uint16_t x, uint16_t y, char c)
{
    if (c < 32 || c > 90)
        c = ' ';

    const uint8_t *bitmap = Font5x7[c - 32];

    for (uint8_t col = 0; col < 5; col++)
    {
        uint8_t line = bitmap[col];

        for (uint8_t row = 0; row < 7; row++)
        {
            if (line & (1 << row))
            {
                LCD_DrawPixel(x + col, y + row, currentColor);
            }
        }
    }
}

void LCD_DrawString(uint16_t x, uint16_t y, const char *text)
{
    while (*text)
    {
        LCD_DrawChar(x, y, *text);
        x += 6;
        text++;
    }
}

void LCD_DisplayString(int x, int y, const char *text)
{
    LCD_DrawString((uint16_t)x, (uint16_t)y, text);
}


void LCD_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) return;

    if ((x + w) > LCD_WIDTH)  w = LCD_WIDTH - x;
    if ((y + h) > LCD_HEIGHT) h = LCD_HEIGHT - y;

    LCD_SetAddressWindow(x, y, x + w - 1, y + h - 1);

    uint8_t data[2];
    data[0] = color >> 8;
    data[1] = color & 0xFF;

    LCD_Select();
    LCD_DC_Data();

    for (uint32_t i = 0; i < w * h; i++)
    {
        HAL_SPI_Transmit(&hspi1, data, 2, HAL_MAX_DELAY);
    }

    LCD_Unselect();
}

void LCD_DrawCharScaled(uint16_t x, uint16_t y, char c, uint8_t scale)
{
    if (c < 32 || c > 90)
        c = ' ';

    const uint8_t *bitmap = Font5x7[c - 32];

    for (uint8_t col = 0; col < 5; col++)
    {
        uint8_t line = bitmap[col];

        for (uint8_t row = 0; row < 7; row++)
        {
            if (line & (1 << row))
            {
                LCD_FillRect(
                    x + col * scale,
                    y + row * scale,
                    scale,
                    scale,
                    currentColor
                );
            }
        }
    }
}

void LCD_DrawStringScaled(uint16_t x, uint16_t y, const char *text, uint8_t scale)
{
    while (*text)
    {
        LCD_DrawCharScaled(x, y, *text, scale);
        x += 6 * scale;
        text++;
    }
}

void LCD_DrawStringCentered(uint16_t y, const char *text, uint8_t scale)
{
    uint16_t len = strlen(text);

    // širina jednog znaka = 5 px + 1 px razmak
    uint16_t width = len * 6 * scale;

    uint16_t x = (LCD_WIDTH - width) / 2;

    LCD_DrawStringScaled(x, y, text, scale);
}

void LCD_DrawLine(uint16_t x0, uint16_t y0,
                  uint16_t x1, uint16_t y1,
                  uint16_t color)
{
    if (y0 == y1)
    {
        for (uint16_t x = x0; x <= x1; x++)
            LCD_DrawPixel(x, y0, color);
    }
    else if (x0 == x1)
    {
        for (uint16_t y = y0; y <= y1; y++)
            LCD_DrawPixel(x0, y, color);
    }
}

void LCD_DrawRect(uint16_t x,
                  uint16_t y,
                  uint16_t w,
                  uint16_t h,
                  uint16_t color)
{
    LCD_DrawLine(x, y, x + w - 1, y, color);
    LCD_DrawLine(x, y + h - 1, x + w - 1, y + h - 1, color);
    LCD_DrawLine(x, y, x, y + h - 1, color);
    LCD_DrawLine(x + w - 1, y, x + w - 1, y + h - 1, color);
}

void LCD_FillCircle(uint16_t x0,
                    uint16_t y0,
                    uint16_t radius,
                    uint16_t color)
{
    int32_t r2 =
        (int32_t)radius *
        (int32_t)radius;

    for (int32_t y = -(int32_t)radius;
         y <= (int32_t)radius;
         y++)
    {
        for (int32_t x = -(int32_t)radius;
             x <= (int32_t)radius;
             x++)
        {
            if ((x * x + y * y) <= r2)
            {
                LCD_DrawPixel(
                    (uint16_t)((int32_t)x0 + x),
                    (uint16_t)((int32_t)y0 + y),
                    color);
            }
        }
    }
}
