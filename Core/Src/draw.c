#include "draw.h"
#include "lcd.h"
#include "main.h"
#include "igre_status.h"
#include "game1.h"
#include "game2.h"
#include "game3.h"
#include <stdio.h>
#include "rezultati.h"

static void DrawMenuItem(uint8_t index,
                         const char *text,
                         uint8_t selected);

/* ------------------------------------------------------------------------- */

void DrawBootScreen(void)
{
    LCD_Clear(BLACK);

    LCD_DrawRect(10, 10, 220, 300, CYAN);

    LCD_SetTextColor(CYAN);
    LCD_DrawStringCentered(70, "HELLO", 3);

    LCD_SetTextColor(GREEN);
    LCD_DrawStringCentered(105, "GAMEBOX", 3);

    LCD_SetTextColor(WHITE);
    LCD_DrawStringCentered(180, "PRESS START", 2);

    /*
     * GREEN tipka = START.
     */
    LCD_FillCircle(
        120,
        225,
        10,
        GREEN);

    LCD_SetTextColor(YELLOW);
    LCD_DrawStringCentered(285, "FIRMWARE V1.0", 1);
}

/* ------------------------------------------------------------------------- */

void DrawMenu(uint8_t selected)
{
    const char *items[] =
    {
        "GAME 1",
        "GAME 2",
        "GAME 3",
        "BEST SCORES"
    };

    LCD_Clear(BLACK);

    LCD_SetTextColor(CYAN);
    LCD_DrawStringCentered(25, "GAMEBOX", 3);

    for (uint8_t i = 0U; i < 4U; i++)
    {
        DrawMenuItem(
            i,
            items[i],
            i == selected);
    }

    /*
     * Donja legenda:
     *
     * BLUE1 = gore
     * BLUE2 = dolje
     * GREEN = START
     */

    /*
     * STRELICA GORE.
     */
    LCD_FillRect(18, 246, 6, 4, WHITE);
    LCD_FillRect(15, 250, 12, 4, WHITE);
    LCD_FillRect(12, 254, 18, 4, WHITE);
    LCD_FillRect(18, 258, 6, 14, WHITE);

    /*
     * BLUE1.
     */
    LCD_FillCircle(
        55,
        258,
        12,
        BLUE);

    LCD_SetTextColor(WHITE);

    LCD_DrawStringScaled(
        53,
        254,
        "1",
        1);

    /*
     * STRELICA DOLJE.
     */
    LCD_FillRect(89, 246, 6, 14, WHITE);
    LCD_FillRect(83, 260, 18, 4, WHITE);
    LCD_FillRect(86, 264, 12, 4, WHITE);
    LCD_FillRect(89, 268, 6, 4, WHITE);

    /*
     * BLUE2.
     */
    LCD_FillCircle(
        125,
        258,
        12,
        BLUE);

    LCD_SetTextColor(WHITE);

    LCD_DrawStringScaled(
        123,
        254,
        "2",
        1);

    /*
     * START.
     */
    LCD_SetTextColor(WHITE);

    LCD_DrawStringScaled(
        155,
        254,
        "START",
        1);

    LCD_FillCircle(
        215,
        258,
        12,
        GREEN);
}

/* ------------------------------------------------------------------------- */

void DrawGame1(void)
{
    LCD_Clear(BLACK);

    LCD_SetTextColor(GREEN);
    LCD_DrawStringCentered(45, "GAME 1", 3);

    LCD_SetTextColor(WHITE);
    LCD_DrawStringCentered(95, "SCORE", 2);

    LCD_DrawStringCentered(190, "TIME", 2);

    DrawGame1Score();
    DrawGame1Interval();
}

/* ------------------------------------------------------------------------- */

void DrawGame1Score(void)
{
    char scoreText[16];

    snprintf(
        scoreText,
        sizeof(scoreText),
        "%u",
        (unsigned int)GetScore());

    /*
     * Briše samo područje rezultata.
     */
    LCD_FillRect(
        20,
        135,
        200,
        40,
        BLACK);

    LCD_SetTextColor(WHITE);
    LCD_DrawStringCentered(145, scoreText, 3);
}

/* ------------------------------------------------------------------------- */

void DrawGame2(void)
{
    LCD_Clear(BLACK);

    LCD_SetTextColor(CYAN);
    LCD_DrawStringCentered(45, "GAME 2", 3);

    LCD_SetTextColor(WHITE);
    LCD_DrawStringCentered(95, "SCORE", 2);

    DrawGame2Score();
}

/* ------------------------------------------------------------------------- */

void DrawGame3(void)
{
    LCD_Clear(BLACK);

    LCD_SetTextColor(YELLOW);
    LCD_DrawStringCentered(45, "GAME 3", 3);

    LCD_SetTextColor(WHITE);
    LCD_DrawStringCentered(95, "SCORE", 2);

    DrawGame3Score();
}

/* ------------------------------------------------------------------------- */

void DrawBestScores(void)
{
    LCD_Clear(BLACK);

    LCD_SetTextColor(WHITE);
    LCD_DrawStringCentered(80, "BEST", 3);
    LCD_DrawStringCentered(115, "SCORES", 3);
}

/* ------------------------------------------------------------------------- */

void DrawGameOver(void)
{
    char buffer[24];

    LCD_Clear(BLACK);

    LCD_SetTextColor(RED);
    LCD_DrawStringCentered(30, "GAME OVER", 3);

    LCD_SetTextColor(WHITE);
    LCD_DrawStringCentered(80, "SCORE", 2);

    snprintf(
        buffer,
        sizeof(buffer),
        "%u",
        (unsigned int)GetFinalScore());

    LCD_SetTextColor(YELLOW);
    LCD_DrawStringCentered(110, buffer, 3);

    /*
     * GAME 1 HIGH SCORE.
     */
    if ((GetSelectedGame() == 0U) &&
        (Game1_IsHighScoreEntryActive() != 0U))
    {
        LCD_SetTextColor(GREEN);
        LCD_DrawStringCentered(
            155,
            "NEW HIGH SCORE!",
            2);

        DrawHighScoreName();

        LCD_SetTextColor(CYAN);
        LCD_DrawStringCentered(
            255,
            "BLUE = LETTER",
            1);

        LCD_SetTextColor(GREEN);
        LCD_DrawStringCentered(
            275,
            "GREEN = OK",
            1);

        return;
    }

    /*
     * GAME 2 HIGH SCORE.
     */
    if ((GetSelectedGame() == 1U) &&
        (Game2_IsHighScoreEntryActive() != 0U))
    {
        LCD_SetTextColor(GREEN);
        LCD_DrawStringCentered(
            155,
            "NEW HIGH SCORE!",
            2);

        DrawHighScoreName();

        LCD_SetTextColor(CYAN);
        LCD_DrawStringCentered(
            255,
            "BLUE = LETTER",
            1);

        LCD_SetTextColor(GREEN);
        LCD_DrawStringCentered(
            275,
            "GREEN = OK",
            1);

        return;
    }
    /*
     * GAME 3 HIGH SCORE.
     */
    if ((GetSelectedGame() == 2U) &&
        (Game3_IsHighScoreEntryActive() != 0U))
    {
        LCD_SetTextColor(GREEN);

        LCD_DrawStringCentered(
            155,
            "NEW HIGH SCORE!",
            2);

        DrawHighScoreName();

        LCD_SetTextColor(CYAN);

        LCD_DrawStringCentered(
            255,
            "BLUE = LETTER",
            1);

        LCD_SetTextColor(GREEN);

        LCD_DrawStringCentered(
            275,
            "GREEN = OK",
            1);

        return;
    }

    LCD_SetTextColor(RED);
    LCD_DrawStringCentered(
        250,
        "RED = BACK",
        2);
}

/* ------------------------------------------------------------------------- */

void DrawHighScoreName(void)
{
    const char *name;
    uint8_t position;
    char letter[2];

    /*
     * Odaberi podatke ovisno o aktivnoj igri.
     */
    if (GetSelectedGame() == 0U)
    {
        name = Game1_GetHighScoreName();
        position = Game1_GetHighScoreNamePosition();
    }
    else if (GetSelectedGame() == 1U)
    {
        name = Game2_GetHighScoreName();
        position = Game2_GetHighScoreNamePosition();
    }
    else
    {
        name = Game3_GetHighScoreName();
        position = Game3_GetHighScoreNamePosition();
    }

    letter[1] = '\0';

    /*
     * Brišemo samo područje inicijala.
     */
    LCD_FillRect(
        50,
        190,
        140,
        45,
        BLACK);

    /*
     * PRVO SLOVO.
     */
    if (position > 0U)
    {
        LCD_SetTextColor(GREEN);
    }
    else
    {
        LCD_SetTextColor(YELLOW);
    }

    letter[0] = name[0];

    LCD_DrawStringScaled(
        75,
        200,
        letter,
        3);

    /*
     * DRUGO SLOVO.
     */
    if (position > 1U)
    {
        LCD_SetTextColor(GREEN);
    }
    else if (position == 1U)
    {
        LCD_SetTextColor(YELLOW);
    }
    else
    {
        LCD_SetTextColor(WHITE);
    }

    letter[0] = name[1];

    LCD_DrawStringScaled(
        110,
        200,
        letter,
        3);

    /*
     * TREĆE SLOVO.
     */
    if (position == 2U)
    {
        LCD_SetTextColor(YELLOW);
    }
    else
    {
        LCD_SetTextColor(WHITE);
    }

    letter[0] = name[2];

    LCD_DrawStringScaled(
        145,
        200,
        letter,
        3);
}

/* ------------------------------------------------------------------------- */

static void DrawMenuItem(uint8_t index,
                         const char *text,
                         uint8_t selected)
{
    uint16_t y =
        90U + ((uint16_t)index * 35U);

    LCD_FillRect(
        30,
        y,
        190,
        18,
        BLACK);

    if (selected != 0U)
    {
        LCD_SetTextColor(GREEN);

        LCD_DrawStringScaled(
            35,
            y,
            ">",
            2);
    }
    else
    {
        LCD_SetTextColor(WHITE);
    }

    LCD_DrawStringScaled(
        60,
        y,
        text,
        2);
}

/* ------------------------------------------------------------------------- */

void DrawGameReady(void)
{
    LCD_Clear(BLACK);

    /*
     * READY?
     */
    LCD_SetTextColor(WHITE);

    LCD_DrawStringCentered(
        70,
        "READY?",
        3);

    /*
     * START.
     */
    LCD_SetTextColor(WHITE);

    LCD_DrawStringCentered(
        145,
        "START",
        2);

    /*
     * Zelena tipka.
     */
    LCD_FillCircle(
        120,
        185,
        12,
        GREEN);

    /*
     * BACK.
     */
    LCD_SetTextColor(WHITE);

    LCD_DrawStringCentered(
        225,
        "BACK",
        2);

    /*
     * Crvena tipka.
     */
    LCD_FillCircle(
        120,
        265,
        12,
        RED);
}

/* ------------------------------------------------------------------------- */

void DrawCountdown(void)
{
    LCD_Clear(BLACK);

    switch (GetCountdownValue())
    {
        case 3:
            LCD_SetTextColor(YELLOW);
            LCD_DrawStringCentered(120, "3", 5);
            break;

        case 2:
            LCD_SetTextColor(YELLOW);
            LCD_DrawStringCentered(120, "2", 5);
            break;

        case 1:
            LCD_SetTextColor(YELLOW);
            LCD_DrawStringCentered(120, "1", 5);
            break;

        case 0:
            LCD_SetTextColor(GREEN);
            LCD_DrawStringCentered(120, "GO", 4);
            break;

        default:
            break;
    }
}

/* ------------------------------------------------------------------------- */

void DrawGame1Interval(void)
{
    char intervalText[20];

    snprintf(
        intervalText,
        sizeof(intervalText),
        "%lu ms",
        (unsigned long)Game1_GetInterval());

    /*
     * Briše samo područje intervala.
     */
    LCD_FillRect(
        20,
        220,
        200,
        35,
        BLACK);

    LCD_SetTextColor(CYAN);

    LCD_DrawStringCentered(
        225,
        intervalText,
        2);
}

/* ------------------------------------------------------------------------- */

void DrawGame1ComboBonus(void)
{
    /*
     * Poruka se crta preko središnjeg dijela Game 1 ekrana.
     */
    LCD_FillRect(
        10,
        75,
        220,
        150,
        BLACK);

    LCD_SetTextColor(YELLOW);
    LCD_DrawStringCentered(
        100,
        "COMBO!",
        3);

    LCD_SetTextColor(GREEN);
    LCD_DrawStringCentered(
        155,
        "+100",
        4);
}

/* ------------------------------------------------------------------------- */

void DrawScores(uint8_t gameIndex)
{
    char buffer[32];
    const GameResult *result;

    LCD_Clear(BLACK);

    /*
     * Naslov.
     */
    LCD_SetTextColor(WHITE);

    LCD_DrawStringCentered(
        20,
        "BEST SCORES",
        3);

    /*
     * Broj igre.
     */
    snprintf(
        buffer,
        sizeof(buffer),
        "GAME %u",
        (unsigned int)(gameIndex + 1U));

    LCD_SetTextColor(CYAN);

    LCD_DrawStringCentered(
        65,
        buffer,
        3);

    /*
     * TOP 3 rezultata.
     */
    for (uint8_t position = 0U;
         position < RESULTS_PER_GAME;
         position++)
    {
        result =
            Rezultati_Get(
                gameIndex,
                position);

        if (result == NULL)
        {
            continue;
        }

        snprintf(
            buffer,
            sizeof(buffer),
            "%u. %s  %lu",
            (unsigned int)(position + 1U),
            result->name,
            (unsigned long)result->score);

        if (position == 0U)
        {
            LCD_SetTextColor(YELLOW);
        }
        else if (position == 1U)
        {
            LCD_SetTextColor(WHITE);
        }
        else
        {
            LCD_SetTextColor(CYAN);
        }

        LCD_DrawStringCentered(
            (uint16_t)(
                115U +
                ((uint16_t)position * 38U)),
            buffer,
            2);
    }

    /*
     * DONJA LEGENDA:
     *
     * BLUE1 = prethodna igra
     * BLUE2 = sljedeća igra
     * RED   = BACK
     */

    /*
     * Strelica GORE.
     */
    LCD_FillRect(14, 247, 6, 14, WHITE);

    LCD_FillRect(14, 239, 6, 4, WHITE);
    LCD_FillRect(11, 243, 12, 4, WHITE);
    LCD_FillRect(8, 247, 18, 4, WHITE);

    /*
     * BLUE1.
     */
    LCD_FillCircle(
        48,
        252,
        12,
        BLUE);

    LCD_SetTextColor(WHITE);

    LCD_DrawStringScaled(
        46,
        248,
        "1",
        1);

    /*
     * Strelica DOLJE.
     */
    LCD_FillRect(82, 239, 6, 14, WHITE);

    LCD_FillRect(76, 253, 18, 4, WHITE);
    LCD_FillRect(79, 257, 12, 4, WHITE);
    LCD_FillRect(82, 261, 6, 4, WHITE);

    /*
     * BLUE2.
     */
    LCD_FillCircle(
        116,
        252,
        12,
        BLUE);

    LCD_SetTextColor(WHITE);

    LCD_DrawStringScaled(
        114,
        248,
        "2",
        1);

    /*
     * BACK.
     */
    LCD_SetTextColor(WHITE);

    LCD_DrawStringScaled(
        148,
        248,
        "BACK",
        1);

    LCD_FillCircle(
        205,
        252,
        12,
        RED);
}

/* ------------------------------------------------------------------------- */

void DrawGame2Score(void)
{
    char scoreText[16];

    snprintf(
        scoreText,
        sizeof(scoreText),
        "%u",
        (unsigned int)GetScore());

    /*
     * Brišemo samo područje rezultata.
     */
    LCD_FillRect(
        20,
        135,
        200,
        40,
        BLACK);

    LCD_SetTextColor(WHITE);

    LCD_DrawStringCentered(
        145,
        scoreText,
        3);
}

/* ------------------------------------------------------------------------- */

void DrawGame2Interval(void)
{
    char intervalText[20];

    snprintf(
        intervalText,
        sizeof(intervalText),
        "%lu ms",
        (unsigned long)Game2_GetInterval());

    /*
     * Debug prikaz intervala.
     * Trenutno se više ne poziva na završnom Game 2 ekranu.
     */
    LCD_FillRect(
        20,
        220,
        200,
        35,
        BLACK);

    LCD_SetTextColor(CYAN);

    LCD_DrawStringCentered(
        225,
        intervalText,
        2);
}

/* ------------------------------------------------------------------------- */

void DrawGame2ComboBonus(void)
{
    /*
     * COMBO privremeno zauzima SCORE područje.
     */
    LCD_FillRect(
        10,
        120,
        220,
        65,
        BLACK);

    LCD_SetTextColor(YELLOW);

    LCD_DrawStringCentered(
        128,
        "COMBO!",
        3);

    LCD_SetTextColor(GREEN);

    LCD_DrawStringCentered(
        162,
        "+50",
        4);
}

/* ------------------------------------------------------------------------- */

void ClearGame2ComboArea(void)
{
    /*
     * Brišemo cijelo područje combo poruke
     * kako ne bi ostali dijelovi velikog teksta.
     */
    LCD_FillRect(
        10,
        115,
        220,
        90,
        BLACK);
}

/* ------------------------------------------------------------------------- */
void DrawGame3Score(void)
{
    char scoreText[16];

    snprintf(
        scoreText,
        sizeof(scoreText),
        "%u",
        (unsigned int)GetScore());

    /*
     * Brišemo samo područje rezultata.
     * Cijeli ekran se ne crta ponovno.
     */
    LCD_FillRect(
        20,
        135,
        200,
        40,
        BLACK);

    LCD_SetTextColor(WHITE);

    LCD_DrawStringCentered(
        145,
        scoreText,
        3);
}
