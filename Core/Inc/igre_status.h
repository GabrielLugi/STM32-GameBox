#ifndef IGRE_STATUS_H
#define IGRE_STATUS_H

#include <stdint.h>

typedef enum
{
    STATE_BOOT = 0,
    STATE_MENU,

    STATE_GAME_READY,
    STATE_GAME_COUNTDOWN,
    STATE_GAME_RUNNING,

    STATE_GAME1,
    STATE_GAME2,
    STATE_GAME3,

    STATE_SCORES,
    STATE_GAMEOVER

} GameState;

/* Game state */
GameState GetGameState(void);
void SetGameState(GameState state);

/* Menu */
uint8_t GetMenuSelected(void);
void SetMenuSelected(uint8_t selected);

/* Score */
uint16_t GetScore(void);
void SetScore(uint16_t score);

/* Final score */
uint16_t GetFinalScore(void);
void SetFinalScore(uint16_t score);

uint8_t GetSelectedGame(void);
void SetSelectedGame(uint8_t game);

uint32_t GetTimer(void);
void SetTimer(uint32_t time);

uint8_t GetCountdownValue(void);
void SetCountdownValue(uint8_t value);

void ResetCountdownSound(void);
uint8_t GetLastCountdownSound(void);
void SetLastCountdownSound(uint8_t value);



#endif
