#include "rezultati.h"

#include "main.h"
#include <string.h>

/* -------------------------------------------------------------------------
 * KONFIGURACIJA
 * ------------------------------------------------------------------------- */

#define GAME_COUNT 3U

/*
 * STM32F446RE FLASH:
 *
 * Sector 7:
 * 0x08060000 - 0x0807FFFF
 * 128 KB
 *
 * Ovaj sektor koristimo za trajno spremanje rezultata.
 */
#define RESULTS_FLASH_ADDRESS   0x08060000U
#define RESULTS_FLASH_SECTOR    FLASH_SECTOR_7

/*
 * MAGIC vrijednost služi za provjeru jesu li u Flashu
 * već spremljeni valjani GameBox podaci.
 *
 * ASCII približno predstavlja "GAME".
 */
#define RESULTS_MAGIC           0x47414D45U

/*
 * Verzija formata podataka.
 *
 * Ako kasnije promijenimo strukturu spremljenih rezultata,
 * možemo povećati verziju i automatski inicijalizirati
 * novi format.
 */
#define RESULTS_VERSION         1U


/* -------------------------------------------------------------------------
 * STRUKTURA PODATAKA U FLASHU
 * ------------------------------------------------------------------------- */

typedef struct
{
    uint32_t magic;
    uint32_t version;

    GameResult results[GAME_COUNT][RESULTS_PER_GAME];

} ResultsFlashData;


/* -------------------------------------------------------------------------
 * RADNA KOPIJA U RAM-u
 * ------------------------------------------------------------------------- */

/*
 * results[igra][pozicija]
 *
 * igra:
 * 0 = Game 1
 * 1 = Game 2
 * 2 = Game 3
 *
 * pozicija:
 * 0 = prvo mjesto
 * 1 = drugo mjesto
 * 2 = treće mjesto
 */
static GameResult results[GAME_COUNT][RESULTS_PER_GAME];


/* =========================================================================
 * PRIVATE FUNCTIONS
 * ========================================================================= */


/* ------------------------------------------------------------------------- */

static void Rezultati_SetDefaults(void)
{
    for (uint8_t game = 0U;
         game < GAME_COUNT;
         game++)
    {
        for (uint8_t position = 0U;
             position < RESULTS_PER_GAME;
             position++)
        {
            strcpy(
                results[game][position].name,
                "---");

            results[game][position].score = 0U;
        }
    }
}


/* ------------------------------------------------------------------------- */

static uint8_t Rezultati_FlashIsValid(void)
{
    const ResultsFlashData *flashData;

    flashData =
        (const ResultsFlashData *)RESULTS_FLASH_ADDRESS;

    /*
     * Provjera MAGIC vrijednosti.
     */
    if (flashData->magic != RESULTS_MAGIC)
    {
        return 0U;
    }

    /*
     * Provjera verzije formata.
     */
    if (flashData->version != RESULTS_VERSION)
    {
        return 0U;
    }

    return 1U;
}


/* ------------------------------------------------------------------------- */

static void Rezultati_LoadFromFlash(void)
{
    const ResultsFlashData *flashData;

    flashData =
        (const ResultsFlashData *)RESULTS_FLASH_ADDRESS;

    memcpy(
        results,
        flashData->results,
        sizeof(results));
}


/* ------------------------------------------------------------------------- */

static uint8_t Rezultati_SaveToFlash(void)
{
    ResultsFlashData flashData;

    FLASH_EraseInitTypeDef eraseInit;
    uint32_t sectorError = 0U;

    uint32_t address;
    const uint32_t *source;
    uint32_t wordCount;

    /*
     * Priprema kompletnog bloka podataka u RAM-u.
     */
    flashData.magic = RESULTS_MAGIC;
    flashData.version = RESULTS_VERSION;

    memcpy(
        flashData.results,
        results,
        sizeof(results));


    /*
     * FLASH:
     * Otključavanje interne Flash memorije.
     */
    HAL_FLASH_Unlock();


    /*
     * Brisanje Sectora 7.
     *
     * STM32 Flash prije novog zapisa mora biti obrisan.
     */
    eraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
    eraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    eraseInit.Sector = RESULTS_FLASH_SECTOR;
    eraseInit.NbSectors = 1U;


    if (HAL_FLASHEx_Erase(
            &eraseInit,
            &sectorError) != HAL_OK)
    {
        HAL_FLASH_Lock();
        return 0U;
    }


    /*
     * Podaci se zapisuju WORD po WORD.
     *
     * FLASH_TYPEPROGRAM_WORD = 32 bita.
     */
    address = RESULTS_FLASH_ADDRESS;

    source = (const uint32_t *)&flashData;

    wordCount =
        (sizeof(ResultsFlashData) + 3U) / 4U;


    for (uint32_t i = 0U;
         i < wordCount;
         i++)
    {
        if (HAL_FLASH_Program(
                FLASH_TYPEPROGRAM_WORD,
                address,
                source[i]) != HAL_OK)
        {
            HAL_FLASH_Lock();
            return 0U;
        }

        address += 4U;
    }


    /*
     * Nakon zapisivanja Flash ponovno zaključavamo.
     */
    HAL_FLASH_Lock();

    return 1U;
}


/* =========================================================================
 * PUBLIC FUNCTIONS
 * ========================================================================= */


/* ------------------------------------------------------------------------- */

void Rezultati_Init(void)
{
    /*
     * Ako Flash već sadrži valjane rezultate,
     * učitaj ih u RAM.
     */
    if (Rezultati_FlashIsValid() != 0U)
    {
        Rezultati_LoadFromFlash();
        return;
    }


    /*
     * Prvo pokretanje ili nevažeći Flash sadržaj.
     */
    Rezultati_SetDefaults();


    /*
     * Odmah spremimo početne vrijednosti kako bi sljedeće
     * pokretanje pronašlo valjani MAGIC i VERSION.
     */
    Rezultati_SaveToFlash();
}


/* ------------------------------------------------------------------------- */

uint8_t Rezultati_IsTop3(uint8_t gameIndex,
                         uint32_t score)
{
    if (gameIndex >= GAME_COUNT)
    {
        return 0U;
    }


    /*
     * Rezultat ulazi u Top 3 ako je veći od trenutačnog
     * rezultata na trećem mjestu.
     */
    if (score >
        results[gameIndex][RESULTS_PER_GAME - 1U].score)
    {
        return 1U;
    }

    return 0U;
}


/* ------------------------------------------------------------------------- */

int8_t Rezultati_Add(uint8_t gameIndex,
                     const char *name,
                     uint32_t score)
{
    int8_t newPosition = -1;


    if ((gameIndex >= GAME_COUNT) ||
        (name == NULL))
    {
        return -1;
    }


    /*
     * Pronađi mjesto novog rezultata.
     */
    for (uint8_t position = 0U;
         position < RESULTS_PER_GAME;
         position++)
    {
        if (score >
            results[gameIndex][position].score)
        {
            newPosition =
                (int8_t)position;

            break;
        }
    }


    /*
     * Rezultat nije ušao u Top 3.
     */
    if (newPosition < 0)
    {
        return -1;
    }


    /*
     * Pomakni slabije rezultate prema dolje.
     */
    for (int8_t position =
             (int8_t)RESULTS_PER_GAME - 1;

         position > newPosition;

         position--)
    {
        results[gameIndex][position] =
            results[gameIndex][position - 1];
    }


    /*
     * Upiši novi rezultat.
     */
    strncpy(
        results[gameIndex][newPosition].name,
        name,
        RESULT_NAME_LENGTH);

    results[gameIndex][newPosition]
        .name[RESULT_NAME_LENGTH] = '\0';

    results[gameIndex][newPosition]
        .score = score;


    /*
     * FLASH:
     * Nakon svake promjene Top 3 tablice spremamo novu
     * verziju u trajnu memoriju.
     */
    Rezultati_SaveToFlash();


    return newPosition;
}


/* ------------------------------------------------------------------------- */

const GameResult *Rezultati_Get(uint8_t gameIndex,
                               uint8_t position)
{
    if ((gameIndex >= GAME_COUNT) ||
        (position >= RESULTS_PER_GAME))
    {
        return NULL;
    }

    return &results[gameIndex][position];
}
