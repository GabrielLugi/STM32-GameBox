#include "game3.h"

#include "main.h"
#include "input.h"
#include "ws2812.h"
#include "buzzer.h"
#include "igre_status.h"
#include "rezultati.h"

/* =========================================================================
 * GAME 3 - TIMING HIT
 *
 * FIZIČKI RASPORED NA GAMEBOXU
 * gledano slijeva nadesno:
 *
 * LED4      LED3      LED2      LED1      LED0
 * BLUE1     BLUE2     GREEN     YELLOW    RED
 *
 * Igra koristi samo GREEN tipku.
 *
 * Svjetlo putuje:
 *
 * LED0 -> LED1 -> LED2 -> LED3 -> LED4
 *                         |
 *                         v
 * LED0 <- LED1 <- LED2 <- LED3 <- LED4
 *
 * odnosno:
 *
 * 0 -> 1 -> 2 -> 3 -> 4 -> 3 -> 2 -> 1 -> 0 ...
 *
 * BODOVANJE:
 *
 * LED2      = +3 boda
 * LED1/LED3 = +1 bod
 * LED0/LED4 = gubitak života
 *
 * TIMER:
 * HAL_GetTick() određuje trenutak pomicanja aktivne LED.
 *
 * INTERRUPT:
 * GREEN tipka dolazi preko EXTI / Input_GetEvent().
 *
 * DMA + TIMER + TRIGGER:
 * WS2812_Show() koristi TIM3_CH1 + DMA.
 * =========================================================================
 */


/* -------------------------------------------------------------------------
 * KONFIGURACIJA
 * ------------------------------------------------------------------------- */

#define GAME3_PLAY_LED_COUNT          5U

#define GAME3_START_LIVES             3U

#define GAME3_START_INTERVAL_MS     250U
#define GAME3_MIN_INTERVAL_MS       100U
#define GAME3_SPEED_STEP_MS          15U
#define GAME3_SPEED_EVERY_HITS        5U

#define GAME3_CENTER_SCORE            3U
#define GAME3_NEAR_SCORE              1U


/* -------------------------------------------------------------------------
 * VARIJABLE
 * ------------------------------------------------------------------------- */

static uint8_t gameRunning = 0U;

static uint8_t lives = GAME3_START_LIVES;

static uint16_t score = 0U;

static uint16_t successfulHits = 0U;


/*
 * Trenutno aktivna LED:
 *
 * 0 - 4
 */
static uint8_t activeLed = 0U;


/*
 * Smjer kretanja:
 *
 * +1 = prema LED4
 * -1 = prema LED0
 */
static int8_t direction = 1;


/*
 * TIMER:
 * vrijeme posljednje promjene LED pozicije.
 */
static uint32_t lastStepTime = 0U;


/*
 * Trenutni interval između dva koraka.
 */
static uint32_t stepInterval =
    GAME3_START_INTERVAL_MS;


/*
 * Nakon jednog GREEN pritiska ne dopuštamo novi
 * pogodak sve dok se aktivna LED ne pomakne
 * na sljedeću poziciju.
 */
static uint8_t hitLocked = 0U;


/*
 * HIGH SCORE stanje.
 */
static uint8_t newHighScore = 0U;

static char highScoreName[4] = "AAA";

static uint8_t highScoreNamePosition = 0U;

static uint8_t highScoreEntryActive = 0U;

static uint32_t highScoreEntryRevision = 0U;


/* =========================================================================
 * PRIVATE FUNCTIONS
 * ========================================================================= */


/* ------------------------------------------------------------------------- */

static void Game3_DrawLives(void)
{
    /*
     * LED5 - LED7:
     *
     * GREEN = život postoji
     * RED   = život izgubljen
     */

    if (lives >= 1U)
    {
        WS2812_SetPixel(
            5U,
            0U,
            255U,
            0U);
    }
    else
    {
        WS2812_SetPixel(
            5U,
            255U,
            0U,
            0U);
    }


    if (lives >= 2U)
    {
        WS2812_SetPixel(
            6U,
            0U,
            255U,
            0U);
    }
    else
    {
        WS2812_SetPixel(
            6U,
            255U,
            0U,
            0U);
    }


    if (lives >= 3U)
    {
        WS2812_SetPixel(
            7U,
            0U,
            255U,
            0U);
    }
    else
    {
        WS2812_SetPixel(
            7U,
            255U,
            0U,
            0U);
    }
}


/* ------------------------------------------------------------------------- */

static void Game3_DrawActiveLed(void)
{
    /*
     * LED0 - LED4 ugasi.
     */
    for (uint8_t i = 0U;
         i < GAME3_PLAY_LED_COUNT;
         i++)
    {
        WS2812_SetPixel(
            i,
            0U,
            0U,
            0U);
    }


    /*
     * Vizualna vrijednost aktivne pozicije:
     *
     * CENTER = GREEN
     * NEAR   = YELLOW
     * EDGE   = RED
     */
    if (activeLed == 2U)
    {
        WS2812_SetPixel(
            activeLed,
            0U,
            255U,
            0U);
    }
    else if ((activeLed == 1U) ||
             (activeLed == 3U))
    {
        WS2812_SetPixel(
            activeLed,
            255U,
            255U,
            0U);
    }
    else
    {
        WS2812_SetPixel(
            activeLed,
            255U,
            0U,
            0U);
    }
}


/* ------------------------------------------------------------------------- */

static void Game3_RefreshLeds(void)
{
    WS2812_Clear();

    Game3_DrawActiveLed();
    Game3_DrawLives();

    /*
     * TIMER + DMA + TRIGGER:
     * TIM3_CH1 + DMA šalju WS2812 signal.
     */
    WS2812_Show();
}


/* ------------------------------------------------------------------------- */

static void Game3_DecreaseInterval(void)
{
    if (stepInterval >
        (GAME3_MIN_INTERVAL_MS +
         GAME3_SPEED_STEP_MS))
    {
        stepInterval -=
            GAME3_SPEED_STEP_MS;
    }
    else
    {
        stepInterval =
            GAME3_MIN_INTERVAL_MS;
    }
}


/* ------------------------------------------------------------------------- */

static void Game3_MoveLed(void)
{
    if (direction > 0)
    {
        if (activeLed < 4U)
        {
            activeLed++;
        }

        if (activeLed >= 4U)
        {
            direction = -1;
        }
    }
    else
    {
        if (activeLed > 0U)
        {
            activeLed--;
        }

        if (activeLed == 0U)
        {
            direction = 1;
        }
    }


    /*
     * Novi LED korak ponovno omogućuje
     * jedan GREEN pritisak.
     */
    hitLocked = 0U;


    Game3_RefreshLeds();
}


/* ------------------------------------------------------------------------- */

static void Game3_End(void)
{
    gameRunning = 0U;

    SetFinalScore(score);

    /*
     * HIGH SCORE:
     *
     * Game 3 koristi indeks 2.
     *
     * Rezultat se još NE sprema.
     * Ako ulazi u Top 3, pokreće se unos inicijala.
     */
    if (Rezultati_IsTop3(
            2U,
            score) != 0U)
    {
        newHighScore = 1U;

        highScoreEntryActive = 1U;

        highScoreNamePosition = 0U;

        highScoreName[0] = 'A';
        highScoreName[1] = 'A';
        highScoreName[2] = 'A';
        highScoreName[3] = '\0';

        highScoreEntryRevision++;
    }
    else
    {
        newHighScore = 0U;

        highScoreEntryActive = 0U;
    }


    BUZZER_Play(
        300U,
        600U);

    SetGameState(
        STATE_GAMEOVER);
}


/* ------------------------------------------------------------------------- */

static void Game3_LoseLife(void)
{
    if (lives > 0U)
    {
        lives--;
    }

    BUZZER_Play(
        350U,
        120U);

    Game3_RefreshLeds();

    if (lives == 0U)
    {
        Game3_End();
    }
}


/* ------------------------------------------------------------------------- */

static void Game3_PerfectHit(void)
{
    score +=
        GAME3_CENTER_SCORE;

    successfulHits++;

    SetScore(score);


    /*
     * Svakih 5 uspješnih pogodaka
     * igra se ubrzava.
     */
    if ((successfulHits %
         GAME3_SPEED_EVERY_HITS) == 0U)
    {
        Game3_DecreaseInterval();
    }


    /*
     * Viši ton za savršen pogodak.
     */
    BUZZER_Play(
        2200U,
        70U);
}


/* ------------------------------------------------------------------------- */

static void Game3_NearHit(void)
{
    score +=
        GAME3_NEAR_SCORE;

    successfulHits++;

    SetScore(score);


    /*
     * I +1 pogodak ulazi u broj
     * uspješnih pogodaka za ubrzavanje.
     */
    if ((successfulHits %
         GAME3_SPEED_EVERY_HITS) == 0U)
    {
        Game3_DecreaseInterval();
    }


    /*
     * Niži ton od savršenog pogotka.
     */
    BUZZER_Play(
        1500U,
        50U);
}


/* ------------------------------------------------------------------------- */

static void Game3_ProcessHit(void)
{
    /*
     * Samo jedan pogodak po jednom LED koraku.
     */
    if (hitLocked != 0U)
    {
        return;
    }

    hitLocked = 1U;


    /*
     * CENTER.
     */
    if (activeLed == 2U)
    {
        Game3_PerfectHit();
    }

    /*
     * SUSJEDNE LED.
     */
    else if ((activeLed == 1U) ||
             (activeLed == 3U))
    {
        Game3_NearHit();
    }

    /*
     * RUBOVI.
     */
    else
    {
        Game3_LoseLife();
    }
}


/* =========================================================================
 * PUBLIC FUNCTIONS
 * ========================================================================= */


/* ------------------------------------------------------------------------- */

void Game3_Init(void)
{
    gameRunning = 1U;

    lives =
        GAME3_START_LIVES;

    score = 0U;

    successfulHits = 0U;

    stepInterval =
        GAME3_START_INTERVAL_MS;


    activeLed = 0U;

    direction = 1;

    hitLocked = 0U;


    /*
     * HIGH SCORE reset.
     */
    newHighScore = 0U;

    highScoreName[0] = 'A';
    highScoreName[1] = 'A';
    highScoreName[2] = 'A';
    highScoreName[3] = '\0';

    highScoreNamePosition = 0U;

    highScoreEntryActive = 0U;

    highScoreEntryRevision = 0U;


    SetScore(0U);
    SetFinalScore(0U);


    /*
     * TIMER:
     * početak prvog LED intervala.
     */
    lastStepTime =
        HAL_GetTick();


    Game3_RefreshLeds();
}


/* ------------------------------------------------------------------------- */

void Game3_Update(void)
{
    InputEvent event;
    uint32_t now;


    if (gameRunning == 0U)
    {
        return;
    }


    now =
        HAL_GetTick();


    /*
     * TIMER:
     * pomicanje aktivne LED.
     */
    if ((now -
         lastStepTime) >=
        stepInterval)
    {
        /*
         * Zadržavamo fiksni ritam.
         */
        lastStepTime +=
            stepInterval;

        Game3_MoveLed();
    }


    /*
     * INTERRUPT:
     * Game 3 koristi samo GREEN tipku.
     */
    event =
        Input_GetEvent();


    if (event != INPUT_GREEN)
    {
        return;
    }


    Game3_ProcessHit();
}


/* ------------------------------------------------------------------------- */

void Game3_Abort(void)
{
    gameRunning = 0U;

    hitLocked = 0U;

    highScoreEntryActive = 0U;

    WS2812_Clear();
    WS2812_Show();

    BUZZER_Off();
}


/* ------------------------------------------------------------------------- */

uint32_t Game3_GetInterval(void)
{
    return stepInterval;
}


/* ------------------------------------------------------------------------- */

uint8_t Game3_IsNewHighScore(void)
{
    return newHighScore;
}


/* ------------------------------------------------------------------------- */

uint8_t Game3_IsHighScoreEntryActive(void)
{
    return highScoreEntryActive;
}


/* ------------------------------------------------------------------------- */

const char *Game3_GetHighScoreName(void)
{
    return highScoreName;
}


/* ------------------------------------------------------------------------- */

uint8_t Game3_GetHighScoreNamePosition(void)
{
    return highScoreNamePosition;
}


/* ------------------------------------------------------------------------- */

uint32_t Game3_GetHighScoreEntryRevision(void)
{
    return highScoreEntryRevision;
}


/* ------------------------------------------------------------------------- */

void Game3_HighScoreInput(InputEvent event)
{
    char *letter;

    if (highScoreEntryActive == 0U)
    {
        return;
    }


    letter =
        &highScoreName[highScoreNamePosition];


    /*
     * BLUE1:
     * A -> B -> C ... -> Z -> A
     */
    if (event == INPUT_BLUE1)
    {
        if (*letter >= 'Z')
        {
            *letter = 'A';
        }
        else
        {
            (*letter)++;
        }

        highScoreEntryRevision++;
    }


    /*
     * BLUE2:
     * A -> Z -> Y ... -> B -> A
     */
    else if (event == INPUT_BLUE2)
    {
        if (*letter <= 'A')
        {
            *letter = 'Z';
        }
        else
        {
            (*letter)--;
        }

        highScoreEntryRevision++;
    }


    /*
     * GREEN:
     * potvrda slova.
     */
    else if (event == INPUT_GREEN)
    {
        if (highScoreNamePosition < 2U)
        {
            highScoreNamePosition++;

            highScoreEntryRevision++;
        }
        else
        {
            /*
             * GAME 3 = indeks 2.
             *
             * Nakon potvrde trećeg slova
             * rezultat se sprema u Top 3 i Flash.
             */
            Rezultati_Add(
                2U,
                highScoreName,
                score);

            highScoreEntryActive = 0U;

            newHighScore = 0U;

            highScoreEntryRevision++;
        }
    }


    /*
     * RED:
     * povratak na prethodno slovo.
     */
    else if (event == INPUT_RED)
    {
        if (highScoreNamePosition > 0U)
        {
            highScoreNamePosition--;

            highScoreEntryRevision++;
        }
    }
}
