#ifndef GAME1_H
#define GAME1_H

#include <stdint.h>
#include "input.h"

void Game1_Init(void);
void Game1_Update(void);
void Game1_Abort(void);

uint16_t Game1_GetScore(void);
uint32_t Game1_GetInterval(void);

uint8_t Game1_IsComboMessageActive(void);
uint32_t Game1_GetCombo(void);

uint8_t Game1_IsNewHighScore(void);

uint8_t Game1_IsNewHighScore(void);

uint8_t Game1_IsHighScoreEntryActive(void);

const char *Game1_GetHighScoreName(void);

uint8_t Game1_GetHighScoreNamePosition(void);

uint32_t Game1_GetHighScoreEntryRevision(void);

void Game1_HighScoreInput(InputEvent event);

#endif
