#ifndef DRAW_H
#define DRAW_H

#include <stdint.h>

void DrawBootScreen(void);

void DrawMenu(uint8_t selected);

void DrawGameReady(void);
void DrawCountdown(void);

void DrawGame1(void);
void DrawGame2(void);
void DrawGame3(void);

void DrawBestScores(void);

void DrawGameOver(void);

void DrawGame1Score(void);
void DrawGame1Interval(void);
void DrawGame2Score(void);
void DrawGame2Interval(void);

void DrawGame1ComboBonus(void);

void DrawScores(uint8_t gameIndex);
void DrawHighScoreName(void);

void DrawGame2ComboBonus(void);
void ClearGame2ComboArea(void);

void DrawGame3Score(void);

#endif
