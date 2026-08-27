#ifndef REZULTATI_H
#define REZULTATI_H

#include <stdint.h>

#define RESULTS_PER_GAME    3U
#define RESULT_NAME_LENGTH  3U

typedef struct
{
    char name[RESULT_NAME_LENGTH + 1U];
    uint32_t score;
} GameResult;

/*
 * Inicijalizira rezultate.
 * Za sada se rezultati čuvaju samo u RAM-u.
 */
void Rezultati_Init(void);

/*
 * Provjerava ulazi li rezultat u Top 3.
 *
 * Povratna vrijednost:
 * 1 = rezultat ulazi u Top 3
 * 0 = rezultat ne ulazi u Top 3
 */
uint8_t Rezultati_IsTop3(uint8_t gameIndex, uint32_t score);

/*
 * Dodaje rezultat u Top 3 i pomiče slabije rezultate.
 *
 * Povratna vrijednost:
 * 0, 1 ili 2 = pozicija novog rezultata
 * -1 = rezultat nije ušao u Top 3
 */
int8_t Rezultati_Add(uint8_t gameIndex,
                     const char *name,
                     uint32_t score);

/*
 * Vraća jedan rezultat.
 */
const GameResult *Rezultati_Get(uint8_t gameIndex,
                               uint8_t position);

#endif
