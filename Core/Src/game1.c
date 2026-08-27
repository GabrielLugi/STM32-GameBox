#include "game1.h"

#include "main.h"
#include "input.h"
#include "buttons.h"
#include "ws2812.h"
#include "buzzer.h"
#include "igre_status.h"
#include "rezultati.h"

/* =========================================================================
 * GAME 1 - REACTION GAME
 *
 * TIMER:
 * HAL_GetTick() koristi SysTick za:
 * - vrijeme reakcije
 * - ubrzavanje igre
 * - trajanje COMBO poruke
 * - dugo držanje BLUE1 + RED za izlaz
 *
 * INTERRUPT:
 * Tipke generiraju događaje preko EXTI prekida.
 * Game1_Update() događaje čita preko Input_GetEvent().
 *
 * DMA + TIMER + TRIGGER:
 * WS2812 LED-ice koriste TIM3_CH1 + DMA.
 * TIM3 CC1 događaj pokreće DMA prijenos.
 * =========================================================================
 */


/* -------------------------------------------------------------------------
 * KONFIGURACIJA IGRE
 * ------------------------------------------------------------------------- */

#define GAME1_PLAY_LED_COUNT          5U

#define GAME1_START_LIVES             3U

#define GAME1_START_INTERVAL_MS    2000U
#define GAME1_MIN_INTERVAL_MS       800U

#define GAME1_SPEED_STEP_MS         100U
#define GAME1_SPEED_EVERY_HITS        5U

#define GAME1_HIT_SCORE              10U

#define GAME1_COMBO_TARGET           20U
#define GAME1_COMBO_BONUS           100U
#define GAME1_COMBO_MESSAGE_MS       700U

#define GAME1_EXIT_HOLD_MS          1500U


/* -------------------------------------------------------------------------
 * VARIJABLE IGRE
 * ------------------------------------------------------------------------- */

static uint8_t lives = GAME1_START_LIVES;

static uint32_t score = 0U;
static uint32_t combo = 0U;
static uint32_t totalHits = 0U;

static uint8_t activeLed = 0U;

static uint32_t ledInterval = GAME1_START_INTERVAL_MS;
static uint32_t ledStartTime = 0U;

static uint8_t gameRunning = 0U;


/*
 * Pseudo-random generator.
 */
static uint32_t randomState = 1U;


/*
 * BLUE1 + RED dugo držanje za izlaz.
 */
static uint8_t exitHoldActive = 0U;
static uint32_t exitHoldStart = 0U;


/*
 * COMBO poruka.
 */
static uint8_t comboMessageActive = 0U;
static uint32_t comboMessageStart = 0U;

static uint8_t newHighScore = 0U;
static char highScoreName[4] = "AAA";
static uint8_t highScoreNamePosition = 0U;
static uint8_t highScoreEntryActive = 0U;
static uint32_t highScoreEntryRevision = 0U;


/* =========================================================================
 * PRIVATE FUNCTIONS
 * ========================================================================= */


/* ------------------------------------------------------------------------- */

static uint32_t Game1_Random(void)
{
    /*
     * Jednostavni LCG pseudo-random generator.
     */
    randomState =
        (randomState * 1664525UL) +
        1013904223UL;

    return randomState;
}


/* ------------------------------------------------------------------------- */

static InputEvent Game1_ExpectedInput(uint8_t led)
{
    /*
     * Fizički raspored LED-ica:
     *
     * LED0 -> RED
     * LED1 -> YELLOW
     * LED2 -> GREEN
     * LED3 -> BLUE2
     * LED4 -> BLUE1
     */

    switch (led)
    {
        case 0U:
            return INPUT_RED;

        case 1U:
            return INPUT_YELLOW;

        case 2U:
            return INPUT_GREEN;

        case 3U:
            return INPUT_BLUE2;

        case 4U:
            return INPUT_BLUE1;

        default:
            return INPUT_NONE;
    }
}


/* ------------------------------------------------------------------------- */

static void Game1_DrawLives(void)
{
    /*
     * LED5, LED6 i LED7 predstavljaju tri života.
     *
     * GREEN = život postoji
     * RED   = život izgubljen
     */

    if (lives >= 1U)
    {
        WS2812_SetPixel(5U, 0U, 255U, 0U);
    }
    else
    {
        WS2812_SetPixel(5U, 255U, 0U, 0U);
    }


    if (lives >= 2U)
    {
        WS2812_SetPixel(6U, 0U, 255U, 0U);
    }
    else
    {
        WS2812_SetPixel(6U, 255U, 0U, 0U);
    }


    if (lives >= 3U)
    {
        WS2812_SetPixel(7U, 0U, 255U, 0U);
    }
    else
    {
        WS2812_SetPixel(7U, 255U, 0U, 0U);
    }
}


/* ------------------------------------------------------------------------- */

static void Game1_DrawActiveLed(void)
{
    /*
     * Aktivna LED odgovara boji fizičke tipke.
     *
     * LED0 = RED
     * LED1 = YELLOW
     * LED2 = GREEN
     * LED3 = BLUE2 / CYAN
     * LED4 = BLUE1 / BLUE
     */

    switch (activeLed)
    {
        case 0U:
            WS2812_SetPixel(
                0U,
                255U,
                0U,
                0U);
            break;


        case 1U:
            WS2812_SetPixel(
                1U,
                255U,
                255U,
                0U);
            break;


        case 2U:
            WS2812_SetPixel(
                2U,
                0U,
                255U,
                0U);
            break;


        case 3U:
            WS2812_SetPixel(
                3U,
                0U,
                255U,
                255U);
            break;


        case 4U:
            WS2812_SetPixel(
                4U,
                0U,
                0U,
                255U);
            break;


        default:
            break;
    }
}


/* ------------------------------------------------------------------------- */

static void Game1_RefreshLeds(void)
{
    WS2812_Clear();

    Game1_DrawActiveLed();
    Game1_DrawLives();

    /*
     * TIMER + DMA + TRIGGER:
     *
     * WS2812_Show() koristi TIM3_CH1.
     * TIM3 CC1 događaj generira DMA request.
     * DMA šalje PWM vrijednosti u TIM3->CCR1.
     */
    WS2812_Show();
}


/* ------------------------------------------------------------------------- */

static void Game1_ShowComboPauseLeds(void)
{
    /*
     * Tijekom COMBO poruke ugasimo LED0-LED4,
     * ali LED5-LED7 i dalje prikazuju živote.
     */

    WS2812_Clear();

    Game1_DrawLives();

    /*
     * TIMER + DMA + TRIGGER:
     * osvježavanje WS2812 lanca.
     */
    WS2812_Show();
}


/* ------------------------------------------------------------------------- */

static void Game1_SelectNextLed(void)
{
    uint8_t newLed;

    /*
     * Biramo jednu od LED0-LED4.
     *
     * Ne dopuštamo da ista LED bude dva puta zaredom.
     */
    do
    {
        newLed =
            (uint8_t)(Game1_Random() %
                      GAME1_PLAY_LED_COUNT);

    } while (newLed == activeLed);


    activeLed = newLed;

    /*
     * TIMER:
     * Od ovog trenutka počinje vrijeme reakcije.
     */
    ledStartTime = HAL_GetTick();

    Game1_RefreshLeds();
}


/* ------------------------------------------------------------------------- */

static uint8_t Game1_CheckExitCombination(void)
{
    uint8_t bluePressed;
    uint8_t redPressed;

    bluePressed =
        Button_IsPressed(BUTTON_BLUE1);

    redPressed =
        Button_IsPressed(BUTTON_RED);


    if ((bluePressed != 0U) &&
        (redPressed != 0U))
    {
        if (exitHoldActive == 0U)
        {
            exitHoldActive = 1U;

            /*
             * TIMER:
             * početak dugog držanja tipki.
             */
            exitHoldStart = HAL_GetTick();
        }
        else
        {
            if ((HAL_GetTick() - exitHoldStart) >=
                GAME1_EXIT_HOLD_MS)
            {
                exitHoldActive = 0U;
                exitHoldStart = 0U;

                return 1U;
            }
        }
    }
    else
    {
        exitHoldActive = 0U;
        exitHoldStart = 0U;
    }

    return 0U;
}


/* ------------------------------------------------------------------------- */

static void Game1_End(void)
{
    gameRunning = 0U;

    SetFinalScore(score);

    /*
     * HIGH SCORE:
     * Rezultat se još NE sprema.
     * Ako ulazi u Top 3, aktivira se unos imena.
     */
    if (Rezultati_IsTop3(0U, score) != 0U)
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

    BUZZER_Play(300U, 600U);

    SetGameState(STATE_GAMEOVER);
}


/* ------------------------------------------------------------------------- */

static void Game1_LoseLife(void)
{
    if (lives > 0U)
    {
        lives--;
    }

    /*
     * Pogreška prekida trenutni combo.
     */
    combo = 0U;


    /*
     * TIMER:
     * niski ton za pogrešku.
     */
    BUZZER_Play(
        350U,
        120U);


    if (lives == 0U)
    {
        /*
         * Prikaži sva tri izgubljena života.
         */
        Game1_RefreshLeds();

        Game1_End();

        return;
    }


    /*
     * Ako još postoje životi,
     * odmah odaberi novu LED.
     */
    Game1_SelectNextLed();
}


/* ------------------------------------------------------------------------- */

static void Game1_CorrectHit(void)
{
    score += GAME1_HIT_SCORE;

    combo++;

    totalHits++;


    /*
     * UBRZAVANJE IGRE:
     *
     * svakih GAME1_SPEED_EVERY_HITS pogodaka
     * vrijeme reakcije smanjuje se za
     * GAME1_SPEED_STEP_MS.
     */
    if ((totalHits %
         GAME1_SPEED_EVERY_HITS) == 0U)
    {
        if (ledInterval >
            (GAME1_MIN_INTERVAL_MS +
             GAME1_SPEED_STEP_MS))
        {
            ledInterval -=
                GAME1_SPEED_STEP_MS;
        }
        else
        {
            ledInterval =
                GAME1_MIN_INTERVAL_MS;
        }
    }


    /*
     * COMBO BONUS.
     */
    if (combo >= GAME1_COMBO_TARGET)
    {
        score += GAME1_COMBO_BONUS;

        combo = 0U;


        /*
         * TIMER:
         * početak prikaza COMBO poruke.
         */
        comboMessageActive = 1U;
        comboMessageStart = HAL_GetTick();


        /*
         * Poseban zvuk za combo.
         */
        BUZZER_Play(
            2400U,
            180U);


        SetScore(score);


        /*
         * Ne biramo novu LED dok traje COMBO poruka.
         */
        Game1_ShowComboPauseLeds();

        return;
    }


    /*
     * Normalan pogodak.
     */
    BUZZER_Play(
        1700U,
        60U);


    SetScore(score);


    Game1_SelectNextLed();
}


/* =========================================================================
 * PUBLIC FUNCTIONS
 * ========================================================================= */


/* ------------------------------------------------------------------------- */

void Game1_Init(void)
{
    lives = GAME1_START_LIVES;

    score = 0U;
    combo = 0U;
    totalHits = 0U;

    ledInterval =
        GAME1_START_INTERVAL_MS;

    ledStartTime = 0U;


    exitHoldActive = 0U;
    exitHoldStart = 0U;


    comboMessageActive = 0U;
    comboMessageStart = 0U;


    gameRunning = 1U;

    newHighScore = 0U;
    highScoreName[0] = 'A';
    highScoreName[1] = 'A';
    highScoreName[2] = 'A';
    highScoreName[3] = '\0';

    highScoreNamePosition = 0U;
    highScoreEntryActive = 0U;
    highScoreEntryRevision = 0U;


    /*
     * Seed za pseudo-random generator.
     *
     * HAL_GetTick() daje različito stanje ovisno
     * o trenutku pokretanja igre.
     */
    randomState =
        HAL_GetTick();

    if (randomState == 0U)
    {
        randomState = 1U;
    }


    SetScore(0U);


    /*
     * activeLed postavljamo izvan normalnog raspona
     * kako bi prvi random izbor sigurno bio prihvaćen.
     */
    activeLed = 255U;


    Game1_SelectNextLed();
}


/* ------------------------------------------------------------------------- */

void Game1_Update(void)
{
    InputEvent event;


    if (gameRunning == 0U)
    {
        return;
    }


    /*
     * BLUE1 + RED:
     * dugo držanje vraća korisnika u glavni izbornik.
     */
    if (Game1_CheckExitCombination() != 0U)
    {
        Game1_Abort();

        SetGameState(STATE_MENU);

        return;
    }


    /*
     * COMBO PAUZA
     *
     * Dok je COMBO poruka aktivna:
     * - nema timeouta
     * - nema novog pogotka
     * - nema nove aktivne LED
     */
    if (comboMessageActive != 0U)
    {
        if ((HAL_GetTick() -
             comboMessageStart) >=
            GAME1_COMBO_MESSAGE_MS)
        {
            /*
             * Ne završavaj COMBO pauzu dok igrač
             * još drži neku tipku.
             */
            if ((Button_IsPressed(BUTTON_BLUE1) != 0U) ||
                (Button_IsPressed(BUTTON_BLUE2) != 0U) ||
                (Button_IsPressed(BUTTON_GREEN) != 0U) ||
                (Button_IsPressed(BUTTON_YELLOW) != 0U) ||
                (Button_IsPressed(BUTTON_RED) != 0U))
            {
                return;
            }


            comboMessageActive = 0U;
            comboMessageStart = 0U;


            /*
             * Tek sada počinje nova runda.
             */
            Game1_SelectNextLed();
        }

        return;
    }


    /*
     * TIMER:
     * Provjera vremena reakcije.
     */
    if ((HAL_GetTick() -
         ledStartTime) >=
        ledInterval)
    {
        Game1_LoseLife();

        return;
    }


    /*
     * INTERRUPT:
     *
     * InputEvent nastaje preko EXTI prekida tipke.
     */
    event = Input_GetEvent();


    if (event == INPUT_NONE)
    {
        return;
    }


    /*
     * Ako igrač započinje kombinaciju
     * BLUE1 + RED za izlaz, događaj ne smijemo
     * odmah računati kao pogrešan pogodak.
     */
    if ((Button_IsPressed(BUTTON_BLUE1) != 0U) &&
        (Button_IsPressed(BUTTON_RED) != 0U))
    {
        return;
    }


    /*
     * Provjera pogodaka.
     */
    if (event ==
        Game1_ExpectedInput(activeLed))
    {
        Game1_CorrectHit();
    }
    else
    {
        Game1_LoseLife();
    }
}


/* ------------------------------------------------------------------------- */

void Game1_Abort(void)
{
    gameRunning = 0U;

    exitHoldActive = 0U;
    exitHoldStart = 0U;

    comboMessageActive = 0U;
    comboMessageStart = 0U;

    WS2812_Clear();
    WS2812_Show();
}


/* ------------------------------------------------------------------------- */

uint32_t Game1_GetInterval(void)
{
    return ledInterval;
}


/* ------------------------------------------------------------------------- */

uint8_t Game1_IsComboMessageActive(void)
{
    return comboMessageActive;
}


/* ------------------------------------------------------------------------- */

uint32_t Game1_GetCombo(void)
{
    return combo;
}

/* ------------------------------------------------------------------------- */

uint8_t Game1_IsNewHighScore(void)
{
    return newHighScore;
}

/* ------------------------------------------------------------------------- */

uint8_t Game1_IsHighScoreEntryActive(void)
{
    return highScoreEntryActive;
}

/* ------------------------------------------------------------------------- */

const char *Game1_GetHighScoreName(void)
{
    return highScoreName;
}

/* ------------------------------------------------------------------------- */

uint8_t Game1_GetHighScoreNamePosition(void)
{
    return highScoreNamePosition;
}

/* ------------------------------------------------------------------------- */

uint32_t Game1_GetHighScoreEntryRevision(void)
{
    return highScoreEntryRevision;
}

/* ------------------------------------------------------------------------- */

void Game1_HighScoreInput(InputEvent event)
{
    char *letter;

    if (highScoreEntryActive == 0U)
    {
        return;
    }

    letter = &highScoreName[highScoreNamePosition];

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
     * potvrdi trenutno slovo.
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
             * Treće slovo potvrđeno.
             * Sada rezultat stvarno spremamo u Flash.
             */
            Rezultati_Add(
                0U,
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
