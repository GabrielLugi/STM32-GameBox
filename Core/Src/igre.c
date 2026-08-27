#include "igre.h"
#include "igre_status.h"
#include "input.h"
#include "main.h"
#include "buzzer.h"
#include "game1.h"
#include "game2.h"
#include "game3.h"

#define COUNTDOWN_NO_TONE 255U

static uint8_t pendingCountdownTone = COUNTDOWN_NO_TONE;

/* ------------------------------------------------------------------------- */

static void ResetGameStartFlow(void)
{
    SetCountdownValue(255U);
    SetTimer(0U);
    pendingCountdownTone = COUNTDOWN_NO_TONE;
}

/* ------------------------------------------------------------------------- */

static void StartCountdown(void)
{
    ResetGameStartFlow();

    /*
     * TIMER:
     * SysTick preko HAL_GetTick() označava početak odbrojavanja.
     */
    SetTimer(HAL_GetTick());
    SetCountdownValue(3U);

    /*
     * Ton će se reproducirati u sljedećem prolazu petlje,
     * nakon što ekran prikaže broj 3.
     */
    pendingCountdownTone = 3U;

    SetGameState(STATE_GAME_COUNTDOWN);
}

/* ------------------------------------------------------------------------- */

static void PlayPendingCountdownTone(void)
{
    if (pendingCountdownTone == COUNTDOWN_NO_TONE)
    {
        return;
    }

    /*
     * TIMER:
     * TIM2_CH3 generira PWM za buzzer.
     */
    if (pendingCountdownTone == 0U)
    {
        BUZZER_Play(2200U, 250U);
    }
    else
    {
        BUZZER_Play(1200U, 80U);
    }

    pendingCountdownTone = COUNTDOWN_NO_TONE;
}

/* ------------------------------------------------------------------------- */

void Igre_Init(void)
{
    SetGameState(STATE_BOOT);
    SetMenuSelected(0U);
    SetSelectedGame(0U);

    SetScore(0U);
    SetFinalScore(0U);

    ResetGameStartFlow();
}

/* ------------------------------------------------------------------------- */

void Igre_Update(void)
{
    InputEvent event = INPUT_NONE;

    /*
     * VAŽNO:
     *
     * Tijekom aktivne igre Input_GetEvent() NE čitamo ovdje.
     *
     * Game1_Update() sam obrađuje događaje tipki preko EXTI-a.
     *
     * Da bismo izbjegli dvostruko čitanje istog događaja,
     * Igre_Update() čita tipke samo izvan STATE_GAME_RUNNING.
     */
    if (GetGameState() != STATE_GAME_RUNNING)
    {
        /*
         * INTERRUPT:
         * InputEvent je zabilježen preko EXTI prekida tipke.
         */
        event = Input_GetEvent();
    }

    PlayPendingCountdownTone();

    switch (GetGameState())
    {
    case STATE_BOOT:
    {
        /*
         * Na početnom ekranu samo GREEN
         * ima funkciju START.
         *
         * Ostale tipke se ignoriraju.
         */
        if (event == INPUT_GREEN)
        {
            SetGameState(STATE_MENU);
        }

        break;
    }

        case STATE_MENU:
        {
            if (event == INPUT_BLUE1)
            {
                if (GetMenuSelected() > 0U)
                {
                    SetMenuSelected(
                        GetMenuSelected() - 1U);
                }
            }
            else if (event == INPUT_BLUE2)
            {
                if (GetMenuSelected() < 3U)
                {
                    SetMenuSelected(
                        GetMenuSelected() + 1U);
                }
            }
            else if (event == INPUT_GREEN)
            {
                if (GetMenuSelected() <= 2U)
                {
                    SetSelectedGame(
                        GetMenuSelected());

                    SetGameState(
                        STATE_GAME_READY);
                }
                else
                {
                    /*
                     * BEST SCORES uvijek se prvo otvara
                     * na rezultatima GAME 1.
                     */
                    SetSelectedGame(0U);

                    SetGameState(
                        STATE_SCORES);
                }
            }

            break;
        }

        case STATE_GAME_READY:
        {
            /*
             * Izvan aktivne igre crvena služi
             * za povratak u izbornik.
             */
            if (event == INPUT_RED)
            {
                ResetGameStartFlow();

                BUZZER_Off();

                SetGameState(
                    STATE_MENU);
            }
            else if (event == INPUT_GREEN)
            {
                StartCountdown();
            }

            break;
        }

        case STATE_GAME_COUNTDOWN:
        {
            uint32_t elapsed;
            uint8_t value;

            /*
             * Tijekom odbrojavanja crvena
             * vraća u meni.
             */
            if (event == INPUT_RED)
            {
                ResetGameStartFlow();

                BUZZER_Off();

                SetGameState(
                    STATE_MENU);

                break;
            }

            /*
             * TIMER:
             * Neblokirajuće odbrojavanje
             * preko HAL_GetTick().
             */
            elapsed =
                HAL_GetTick() -
                GetTimer();

            if (elapsed < 1000U)
            {
                value = 3U;
            }
            else if (elapsed < 2000U)
            {
                value = 2U;
            }
            else if (elapsed < 3000U)
            {
                value = 1U;
            }
            else if (elapsed < 4000U)
            {
                value = 0U;
            }
            else
            {
                ResetGameStartFlow();

                /*
                 * Nakon završetka GO pokrećemo
                 * odabranu igru.
                 */
                if (GetSelectedGame() == 0U)
                {
                    Game1_Init();
                }
                else if (GetSelectedGame() == 1U)
                {
                    Game2_Init();
                }
                else if (GetSelectedGame() == 2U)
                {
                    Game3_Init();
                }

                SetGameState(
                    STATE_GAME_RUNNING);

                break;
            }

            if (value != GetCountdownValue())
            {
                SetCountdownValue(value);

                pendingCountdownTone =
                    value;
            }

            break;
        }

        case STATE_GAME_RUNNING:
        {
            /*
             * VAŽNO:
             *
             * Ovdje Igre_Update() više NIJE
             * pročitao Input_GetEvent().
             *
             * Game1_Update() dobiva događaj
             * direktno iz input sustava.
             *
             * INTERRUPT:
             * EXTI događaje tipki obrađuje igra.
             */

        	if (GetSelectedGame() == 0U)
        	{
        	    Game1_Update();
        	}
        	else if (GetSelectedGame() == 1U)
        	{
        	    Game2_Update();
        	}
        	else if (GetSelectedGame() == 2U)
        	{
        	    Game3_Update();
        	}
            break;
        }

        case STATE_SCORES:
        {
            /*
             * BLUE1 = prethodna igra
             * BLUE2 = sljedeća igra
             * RED   = povratak u glavni meni
             */

            if (event == INPUT_BLUE1)
            {
                if (GetSelectedGame() > 0U)
                {
                    SetSelectedGame(
                        GetSelectedGame() - 1U);
                }
            }
            else if (event == INPUT_BLUE2)
            {
                if (GetSelectedGame() < 2U)
                {
                    SetSelectedGame(
                        GetSelectedGame() + 1U);
                }
            }
            else if (event == INPUT_RED)
            {
                ResetGameStartFlow();

                SetSelectedGame(0U);

                SetGameState(
                    STATE_MENU);
            }

            break;
        }

        case STATE_GAMEOVER:
        {
            /*
             * GAME 1 - HIGH SCORE unos.
             */
            if ((GetSelectedGame() == 0U) &&
                (Game1_IsHighScoreEntryActive() != 0U))
            {
                Game1_HighScoreInput(event);

                if (Game1_IsHighScoreEntryActive() == 0U)
                {
                    Game1_Abort();
                    ResetGameStartFlow();
                    SetGameState(STATE_MENU);
                }

                break;
            }

            /*
             * GAME 2 - HIGH SCORE unos.
             */
            if ((GetSelectedGame() == 1U) &&
                (Game2_IsHighScoreEntryActive() != 0U))
            {
                Game2_HighScoreInput(event);

                if (Game2_IsHighScoreEntryActive() == 0U)
                {
                    Game2_Abort();
                    ResetGameStartFlow();
                    SetGameState(STATE_MENU);
                }

                break;
            }

            /*
             * GAME 3 - HIGH SCORE unos.
             */
            if ((GetSelectedGame() == 2U) &&
                (Game3_IsHighScoreEntryActive() != 0U))
            {
                Game3_HighScoreInput(event);

                if (Game3_IsHighScoreEntryActive() == 0U)
                {
                    Game3_Abort();
                    ResetGameStartFlow();
                    SetGameState(STATE_MENU);
                }

                break;
            }

            /*
             * Obični GAME OVER bez Top 3 rezultata.
             *
             * RED = povratak u MENU.
             */
            if (event == INPUT_RED)
            {
                if (GetSelectedGame() == 0U)
                {
                    Game1_Abort();
                }
                else if (GetSelectedGame() == 1U)
                {
                    Game2_Abort();
                }
                else if (GetSelectedGame() == 2U)
                {
                    Game3_Abort();
                }

                ResetGameStartFlow();
                SetGameState(STATE_MENU);
            }

            break;
        }

        default:
        {
            break;
        }
    }
}
