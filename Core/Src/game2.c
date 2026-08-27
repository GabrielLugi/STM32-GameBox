#include "game2.h"

#include "main.h"
#include "input.h"
#include "buttons.h"
#include "ws2812.h"
#include "buzzer.h"
#include "igre_status.h"
#include "rezultati.h"

/* =========================================================================
 * GAME 2 - COLOR MATCH
 *
 * FIZIČKI RASPORED NA GAMEBOXU
 * gledano slijeva nadesno:
 *
 * LED4      LED3      LED2      LED1      LED0
 * BLUE1     BLUE2     GREEN     YELLOW    RED
 *
 * REFERENCA ODGOVOR    ODGOVOR   ODGOVOR   ODGOVOR
 *
 * LED5, LED6, LED7 = prikaz života
 *
 * PRAVILO:
 * LED4 prikazuje referentnu boju.
 *
 * Na LED0 - LED3 uvijek postoji točno jedna LED
 * iste boje kao LED4.
 *
 * Igrač mora pritisnuti tipku ispod te LED.
 *
 * TIMER:
 * HAL_GetTick() koristi se za:
 * - fiksni ritam promjene kombinacija
 * - mjerenje 8 s bez točnog pogotka
 * - progresivno ubrzavanje igre
 *
 * INTERRUPT:
 * Tipke se registriraju preko EXTI prekida i Input_GetEvent().
 *
 * DMA + TIMER + TRIGGER:
 * WS2812_Show() koristi TIM3_CH1 + DMA.
 * =========================================================================
 */


/* -------------------------------------------------------------------------
 * KONFIGURACIJA
 * ------------------------------------------------------------------------- */

#define GAME2_REFERENCE_LED            4U
#define GAME2_ANSWER_LED_COUNT         4U

#define GAME2_START_LIVES              3U

#define GAME2_START_INTERVAL_MS      900U
#define GAME2_MIN_INTERVAL_MS        350U
#define GAME2_SPEED_STEP_MS           50U

#define GAME2_SPEED_EVERY_HITS         5U

#define GAME2_INACTIVITY_TIME_MS    8000U

#define GAME2_HIT_SCORE               10U

#define GAME2_COMBO_HITS              5U
#define GAME2_COMBO_BONUS            50U

#define GAME2_COMBO_MESSAGE_MS 650U

/* -------------------------------------------------------------------------
 * BOJE
 * ------------------------------------------------------------------------- */

typedef enum
{
    GAME2_COLOR_BLUE = 0,
    GAME2_COLOR_CYAN,
    GAME2_COLOR_GREEN,
    GAME2_COLOR_YELLOW,
    GAME2_COLOR_RED,

    GAME2_COLOR_COUNT

} Game2Color;


/* -------------------------------------------------------------------------
 * VARIJABLE IGRE
 * ------------------------------------------------------------------------- */

static uint8_t gameRunning = 0U;

static uint8_t lives = GAME2_START_LIVES;

static uint32_t score = 0U;

static uint32_t totalHits = 0U;

/*
 * Broj uzastopnih točnih odgovora.
 * Pogrešan odgovor vraća combo na nulu.
 */
static uint8_t combo = 0U;

static uint32_t changeInterval =
    GAME2_START_INTERVAL_MS;

static uint32_t lastChangeTime = 0U;

static uint32_t lastCorrectHitTime = 0U;

static uint32_t randomState = 1U;

static Game2Color referenceColor =
    GAME2_COLOR_RED;


/*
 * Softverski raspored:
 *
 * answerColors[0] -> LED0 -> RED
 * answerColors[1] -> LED1 -> YELLOW
 * answerColors[2] -> LED2 -> GREEN
 * answerColors[3] -> LED3 -> BLUE2
 */
static Game2Color
    answerColors[GAME2_ANSWER_LED_COUNT];


/*
 * 0 -> LED0 / RED
 * 1 -> LED1 / YELLOW
 * 2 -> LED2 / GREEN
 * 3 -> LED3 / BLUE2
 */
static uint8_t correctAnswer = 0U;


/*
 * Sprječava ponavljanje iste pozicije
 * u dvije uzastopne kombinacije.
 */
static uint8_t previousCorrectAnswer = 255U;

static uint8_t comboMessageActive = 0U;
static uint32_t comboMessageStart = 0U;
static uint32_t comboMessageRevision = 0U;

static uint8_t newHighScore = 0U;

static char highScoreName[4] = "AAA";
static uint8_t highScoreNamePosition = 0U;
static uint8_t highScoreEntryActive = 0U;
static uint32_t highScoreEntryRevision = 0U;

/* =========================================================================
 * PRIVATE FUNCTIONS
 * ========================================================================= */


/* ------------------------------------------------------------------------- */

static uint32_t Game2_Random(void)
{
    randomState =
        (randomState * 1664525UL) +
        1013904223UL;

    return randomState;
}


/* ------------------------------------------------------------------------- */

static uint32_t Game2_RandomHigh(void)
{
    /*
     * Koristimo više bitove LCG generatora.
     *
     * Najniži bitovi LCG-a imaju izraženije pravilne obrasce,
     * pogotovo kod operacija poput % 4.
     */
    return Game2_Random() >> 16U;
}


/* ------------------------------------------------------------------------- */

static void Game2_SetLedColor(uint8_t led,
                              Game2Color color)
{
    switch (color)
    {
        case GAME2_COLOR_BLUE:
            WS2812_SetPixel(
                led,
                0U,
                0U,
                255U);
            break;

        case GAME2_COLOR_CYAN:
            WS2812_SetPixel(
                led,
                0U,
                255U,
                255U);
            break;

        case GAME2_COLOR_GREEN:
            WS2812_SetPixel(
                led,
                0U,
                255U,
                0U);
            break;

        case GAME2_COLOR_YELLOW:
            WS2812_SetPixel(
                led,
                255U,
                255U,
                0U);
            break;

        case GAME2_COLOR_RED:
            WS2812_SetPixel(
                led,
                255U,
                0U,
                0U);
            break;

        default:
            WS2812_SetPixel(
                led,
                0U,
                0U,
                0U);
            break;
    }
}


/* ------------------------------------------------------------------------- */

static void Game2_DrawLives(void)
{
    /*
     * LED5 - LED7 = životi.
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

static void Game2_DrawLeds(void)
{
    WS2812_Clear();

    /*
     * LED0 - LED3:
     * četiri moguća odgovora.
     */
    for (uint8_t i = 0U;
         i < GAME2_ANSWER_LED_COUNT;
         i++)
    {
        Game2_SetLedColor(
            i,
            answerColors[i]);
    }

    /*
     * LED4:
     * referentna boja.
     */
    Game2_SetLedColor(
        GAME2_REFERENCE_LED,
        referenceColor);

    /*
     * LED5 - LED7:
     * životi.
     */
    Game2_DrawLives();

    /*
     * TIMER + DMA + TRIGGER:
     * TIM3_CH1 + DMA šalju podatke WS2812 lancu.
     */
    WS2812_Show();
}


/* ------------------------------------------------------------------------- */

static void Game2_GenerateReferenceColor(void)
{
    Game2Color newColor;

    /*
     * Nova referentna boja nakon pogreške
     * mora biti različita od prethodne.
     */
    do
    {
        newColor =
            (Game2Color)(
                Game2_RandomHigh() %
                GAME2_COLOR_COUNT);
    }
    while (newColor == referenceColor);

    referenceColor = newColor;
}


/* ------------------------------------------------------------------------- */

static void Game2_GenerateCombination(void)
{
    uint8_t newCorrectAnswer;

    /*
     * Biramo novu poziciju točnog odgovora.
     *
     * Koristimo više bitove random generatora
     * da izbjegnemo pravilni uzorak:
     * 0 -> 1 -> 2 -> 3 -> ...
     *
     * Ista pozicija ne smije se ponoviti
     * dvije kombinacije zaredom.
     */
    do
    {
        newCorrectAnswer =
            (uint8_t)(
                Game2_RandomHigh() %
                GAME2_ANSWER_LED_COUNT);
    }
    while (newCorrectAnswer ==
           previousCorrectAnswer);

    correctAnswer =
        newCorrectAnswer;

    previousCorrectAnswer =
        correctAnswer;


    /*
     * Generiranje boja LED0 - LED3.
     */
    for (uint8_t i = 0U;
         i < GAME2_ANSWER_LED_COUNT;
         i++)
    {
        if (i == correctAnswer)
        {
            /*
             * Točno jedna LED dobiva
             * referentnu boju.
             */
            answerColors[i] =
                referenceColor;
        }
        else
        {
            Game2Color color;

            /*
             * Ostale LED-ice ne smiju imati
             * referentnu boju.
             */
            do
            {
                color =
                    (Game2Color)(
                        Game2_RandomHigh() %
                        GAME2_COLOR_COUNT);
            }
            while (color ==
                   referenceColor);

            answerColors[i] =
                color;
        }
    }

    Game2_DrawLeds();
}


/* ------------------------------------------------------------------------- */

static int8_t Game2_EventToAnswer(InputEvent event)
{
    /*
     * FIZIČKI RASPORED NA GAMEBOXU:
     *
     * LED4      LED3      LED2      LED1      LED0
     * BLUE1     BLUE2     GREEN     YELLOW    RED
     *
     * REF       ODGOVOR    ODGOVOR   ODGOVOR   ODGOVOR
     */

    switch (event)
    {
        case INPUT_RED:
            return 0;

        case INPUT_YELLOW:
            return 1;

        case INPUT_GREEN:
            return 2;

        case INPUT_BLUE2:
            return 3;

        default:
            /*
             * BLUE1 pripada referentnoj LED4
             * i nije odgovor.
             */
            return -1;
    }
}


/* ------------------------------------------------------------------------- */

static void Game2_DecreaseInterval(void)
{
    /*
     * Interval se smanjuje za 50 ms,
     * ali nikada ispod 350 ms.
     */
    if (changeInterval >
        (GAME2_MIN_INTERVAL_MS +
         GAME2_SPEED_STEP_MS))
    {
        changeInterval -=
            GAME2_SPEED_STEP_MS;
    }
    else
    {
        changeInterval =
            GAME2_MIN_INTERVAL_MS;
    }
}


/* ------------------------------------------------------------------------- */

static void Game2_End(void)
{
    gameRunning = 0U;

    SetFinalScore(
        (uint16_t)score);

    /*
     * HIGH SCORE:
     * Game 2 koristi indeks 1.
     *
     * Rezultat se NE sprema odmah.
     * Ako ulazi u Top 3, pokreće se unos imena.
     */
    if (Rezultati_IsTop3(1U, score) != 0U)
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

static void Game2_CorrectHit(void)
{
    /*
     * Osnovni pogodak.
     */
    score += GAME2_HIT_SCORE;

    totalHits++;

    /*
     * COMBO:
     * svaki uzastopni točan pogodak
     * povećava combo brojač.
     */
    combo++;

    /*
     * Svakih 5 uzastopnih pogodaka
     * dodajemo +50 bonus bodova.
     *
     * Nakon bonusa combo vraćamo na nulu
     * i počinje novi niz od 5 pogodaka.
     */
    if (combo >= GAME2_COMBO_HITS)
    {
        score += GAME2_COMBO_BONUS;

        combo = 0U;

        comboMessageActive = 1U;
        comboMessageStart = HAL_GetTick();
        comboMessageRevision++;
    }

    SetScore(
        (uint16_t)score);

    /*
     * TIMER:
     * točan pogodak resetira
     * 8-sekundni timer neaktivnosti.
     */
    lastCorrectHitTime =
        HAL_GetTick();

    /*
     * Brzina se i dalje računa prema
     * ukupnom broju pogodaka, ne prema combu.
     *
     * Svakih 5 pogodaka:
     * interval -50 ms.
     */
    if ((totalHits %
         GAME2_SPEED_EVERY_HITS) == 0U)
    {
        Game2_DecreaseInterval();
    }

    /*
     * Zvuk točnog odgovora.
     */
    BUZZER_Play(
        1700U,
        60U);

    /*
     * Ne mijenjamo LED kombinaciju.
     * Ona nastavlja svojim fiksnim ritmom.
     */
}

/* ------------------------------------------------------------------------- */

static void Game2_LoseLife(void)
{
    if (lives > 0U)
    {
        lives--;
    }

    /*
     * Pogreška prekida niz
     * uzastopnih pogodaka.
     */
    combo = 0U;

    /*
     * Zvuk pogreške.
     */
    BUZZER_Play(
        350U,
        120U);


    if (lives == 0U)
    {
        /*
         * Osvježi LED prikaz izgubljenih života.
         */
        Game2_DrawLeds();

        Game2_End();

        return;
    }


    /*
     * Pogreška mijenja referentnu boju.
     */
    Game2_GenerateReferenceColor();


    /*
     * Nakon promjene referentne boje
     * dopuštamo bilo koju novu poziciju.
     */
    previousCorrectAnswer =
        255U;


    /*
     * Nova kombinacija za novi cilj.
     */
    Game2_GenerateCombination();


    /*
     * TIMER:
     * novi puni interval počinje sada.
     */
    lastChangeTime =
        HAL_GetTick();
}


/* =========================================================================
 * PUBLIC FUNCTIONS
 * ========================================================================= */


/* ------------------------------------------------------------------------- */

void Game2_Init(void)
{
    uint32_t now;

    gameRunning = 1U;

    lives = GAME2_START_LIVES;

    score = 0U;

    totalHits = 0U;

    combo = 0U;

    comboMessageActive = 0U;
    comboMessageStart = 0U;
    comboMessageRevision = 0U;

    changeInterval = GAME2_START_INTERVAL_MS;

    previousCorrectAnswer = 255U;

    newHighScore = 0U;

    highScoreName[0] = 'A';
    highScoreName[1] = 'A';
    highScoreName[2] = 'A';
    highScoreName[3] = '\0';

    highScoreNamePosition = 0U;
    highScoreEntryActive = 0U;
    highScoreEntryRevision = 0U;


    now = HAL_GetTick();


    /*
     * Seed pseudo-random generatora.
     */
    randomState = now;

    if (randomState == 0U)
    {
        randomState = 1U;
    }


    /*
     * Početna referentna boja.
     */
    referenceColor =
        (Game2Color)(
            Game2_RandomHigh() %
            GAME2_COLOR_COUNT);


    SetScore(0U);


    /*
     * Prva kombinacija.
     */
    Game2_GenerateCombination();


    /*
     * TIMER:
     * početak ritma i timera neaktivnosti.
     */
    now =
        HAL_GetTick();

    lastChangeTime =
        now;

    lastCorrectHitTime =
        now;
}


/* ------------------------------------------------------------------------- */

void Game2_Update(void)
{
    InputEvent event;
    int8_t selectedAnswer;
    uint32_t now;


    if (gameRunning == 0U)
    {
        return;
    }


    now = HAL_GetTick();

    /*
     * COMBO poruka traje 600 ms,
     * ali NE zaustavlja igru.
     */
    if (comboMessageActive != 0U)
    {
        if ((now - comboMessageStart) >=
            GAME2_COMBO_MESSAGE_MS)
        {
            comboMessageActive = 0U;
            comboMessageStart = 0U;
            comboMessageRevision++;
        }
    }


    /*
     * TIMER:
     * fiksni ritam izmjene kombinacija.
     */
    if ((now -
         lastChangeTime) >=
        changeInterval)
    {
        /*
         * Zadržavamo vremenski ritam
         * neovisno o pritiscima tipki.
         */
        lastChangeTime +=
            changeInterval;

        Game2_GenerateCombination();
    }


    /*
     * TIMER:
     * 8 sekundi bez točnog pogotka
     * -> dodatno ubrzavanje.
     */
    if ((now -
         lastCorrectHitTime) >=
        GAME2_INACTIVITY_TIME_MS)
    {
        Game2_DecreaseInterval();

        /*
         * Novi 8-sekundni period.
         */
        lastCorrectHitTime =
            now;
    }


    /*
     * INTERRUPT:
     * događaj tipke iz EXTI sustava.
     */
    event =
        Input_GetEvent();


    if (event == INPUT_NONE)
    {
        return;
    }


    selectedAnswer =
        Game2_EventToAnswer(event);


    /*
     * BLUE1 pripada referentnoj LED4.
     */
    if (selectedAnswer < 0)
    {
        return;
    }


    /*
     * TOČAN ODGOVOR.
     */
    if ((uint8_t)selectedAnswer ==
        correctAnswer)
    {
        Game2_CorrectHit();
    }
    else
    {
        /*
         * POGREŠAN ODGOVOR.
         */
        Game2_LoseLife();
    }
}


/* ------------------------------------------------------------------------- */

void Game2_Abort(void)
{
    gameRunning = 0U;

    WS2812_Clear();
    WS2812_Show();

    BUZZER_Off();
}


/* ------------------------------------------------------------------------- */

uint32_t Game2_GetInterval(void)
{
    return changeInterval;
}

/* ------------------------------------------------------------------------- */

uint8_t Game2_IsComboMessageActive(void)
{
    return comboMessageActive;
}

uint32_t Game2_GetComboMessageRevision(void)
{
    return comboMessageRevision;
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
uint8_t Game2_IsNewHighScore(void)
{
    return newHighScore;
}

/* ------------------------------------------------------------------------- */

uint8_t Game2_IsHighScoreEntryActive(void)
{
    return highScoreEntryActive;
}

/* ------------------------------------------------------------------------- */

const char *Game2_GetHighScoreName(void)
{
    return highScoreName;
}

/* ------------------------------------------------------------------------- */

uint8_t Game2_GetHighScoreNamePosition(void)
{
    return highScoreNamePosition;
}

/* ------------------------------------------------------------------------- */

uint32_t Game2_GetHighScoreEntryRevision(void)
{
    return highScoreEntryRevision;
}

/* ------------------------------------------------------------------------- */

void Game2_HighScoreInput(InputEvent event)
{
    char *letter;

    if (highScoreEntryActive == 0U)
    {
        return;
    }

    letter =
        &highScoreName[highScoreNamePosition];

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
             * Game 2 = indeks 1.
             */
            Rezultati_Add(
                1U,
                highScoreName,
                score);

            highScoreEntryActive = 0U;
            newHighScore = 0U;

            highScoreEntryRevision++;
        }
    }
    else if (event == INPUT_RED)
    {
        if (highScoreNamePosition > 0U)
        {
            highScoreNamePosition--;
            highScoreEntryRevision++;
        }
    }
}
