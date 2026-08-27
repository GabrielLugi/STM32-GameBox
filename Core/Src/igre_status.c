#include "igre_status.h"

static GameState currentState = STATE_BOOT;

static uint8_t menuSelected = 0;

static uint16_t score = 0;

static uint16_t finalScore = 0;

static uint8_t selectedGame = 0;

static uint32_t gameTimer = 0;

/*------------------------------------------------*/

GameState GetGameState(void)
{
    return currentState;
}

void SetGameState(GameState state)
{
    currentState = state;
}

/*------------------------------------------------*/

uint8_t GetMenuSelected(void)
{
    return menuSelected;
}

void SetMenuSelected(uint8_t selected)
{
    menuSelected = selected;
}

/*------------------------------------------------*/

uint16_t GetScore(void)
{
    return score;
}

void SetScore(uint16_t s)
{
    score = s;
}

/*------------------------------------------------*/

uint16_t GetFinalScore(void)
{
    return finalScore;
}

void SetFinalScore(uint16_t s)
{
    finalScore = s;
}

uint8_t GetSelectedGame(void)
{
    return selectedGame;
}

void SetSelectedGame(uint8_t game)
{
    selectedGame = game;
}

uint32_t GetTimer(void)
{
    return gameTimer;
}

void SetTimer(uint32_t time)
{
    gameTimer = time;
}

static uint8_t countdownValue = 255;

uint8_t GetCountdownValue(void)
{
    return countdownValue;
}

void SetCountdownValue(uint8_t value)
{
    countdownValue = value;
}
