#ifndef GAME3_H
#define GAME3_H

#include <stdint.h>
#include "input.h"

/*
 * GAME 3 - TIMING HIT
 */

void Game3_Init(void);
void Game3_Update(void);
void Game3_Abort(void);

uint32_t Game3_GetInterval(void);

/*
 * HIGH SCORE
 */
uint8_t Game3_IsNewHighScore(void);

uint8_t Game3_IsHighScoreEntryActive(void);

const char *Game3_GetHighScoreName(void);

uint8_t Game3_GetHighScoreNamePosition(void);

uint32_t Game3_GetHighScoreEntryRevision(void);

void Game3_HighScoreInput(InputEvent event);

#endif
