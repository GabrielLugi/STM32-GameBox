#ifndef GAME2_H
#define GAME2_H

#include <stdint.h>
#include "input.h"

void Game2_Init(void);
void Game2_Update(void);
void Game2_Abort(void);

uint32_t Game2_GetInterval(void);

uint8_t Game2_IsComboMessageActive(void);
uint32_t Game2_GetComboMessageRevision(void);

uint8_t Game2_IsNewHighScore(void);

uint8_t Game2_IsHighScoreEntryActive(void);

const char *Game2_GetHighScoreName(void);

uint8_t Game2_GetHighScoreNamePosition(void);

uint32_t Game2_GetHighScoreEntryRevision(void);

void Game2_HighScoreInput(InputEvent event);

#endif
