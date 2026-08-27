#include "ekran.h"
#include "draw.h"
#include "igre_status.h"
#include "game1.h"
#include "game2.h"
#include "game3.h"

void Ekran_Init(void)
{
}

void Ekran(void)
{
    static GameState lastState = (GameState)255;
    static uint8_t lastSelected = 255U;
    static uint8_t lastCountdown = 255U;
    static uint16_t lastScore = 0xFFFFU;
    static uint32_t lastHighScoreRevision = 0xFFFFFFFFU;
    static uint32_t lastGame2HighScoreRevision = 0xFFFFFFFFU;
    static uint32_t lastGame3HighScoreRevision = 0xFFFFFFFFU;
    static uint32_t lastGame1Interval = 0xFFFFFFFFU;
    static uint32_t lastGame2ComboRevision = 0xFFFFFFFFU;
    static uint8_t lastScoresGame = 255U;

    uint32_t currentGame1Interval = Game1_GetInterval();

    GameState currentState = GetGameState();
    uint8_t currentSelected = GetMenuSelected();
    uint8_t currentCountdown = GetCountdownValue();
    uint16_t currentScore = GetScore();
    uint32_t currentGame2HighScoreRevision = Game2_GetHighScoreEntryRevision();
    uint32_t currentGame3HighScoreRevision = Game3_GetHighScoreEntryRevision();
    static uint8_t lastComboMessage = 0U;
    uint8_t currentComboMessage = Game1_IsComboMessageActive();
    uint32_t currentGame2ComboRevision = Game2_GetComboMessageRevision();

    /*
     * TIMER:
     * Countdown vrijednost se mijenja prema HAL_GetTick() logici
     * iz igre.c, a ekran se crta samo kada se broj promijeni.
     */
    if (currentState == STATE_GAME_COUNTDOWN)
    {
        if ((currentState == lastState) &&
            (currentCountdown == lastCountdown))
        {
            return;
        }

        lastState = currentState;
        lastCountdown = currentCountdown;

        DrawCountdown();
        return;
    }

    /*
     * Tijekom aktivne igre ekran se osvježava kada se promijeni rezultat.
     */
    if (currentState == STATE_GAME_RUNNING)
    {
        /*
         * Prvi ulazak u aktivnu igru.
         */
        if (currentState != lastState)
        {
            lastState = currentState;
            lastScore = currentScore;
            lastGame1Interval = currentGame1Interval;
            lastComboMessage = currentComboMessage;
            lastCountdown = 255U;
            lastGame2ComboRevision = currentGame2ComboRevision;

            switch (GetSelectedGame())
            {
                case 0U:
                    DrawGame1();
                    break;

                case 1U:
                    DrawGame2();
                    break;

                case 2U:
                    DrawGame3();
                    break;

                default:
                    break;
            }

            return;
        }

        if (GetSelectedGame() == 0U)
        {
            /*
             * COMBO poruka upravo je aktivirana.
             */
            if ((currentComboMessage != 0U) &&
                (lastComboMessage == 0U))
            {
                lastComboMessage = 1U;
                DrawGame1ComboBonus();
                return;
            }

            /*
             * COMBO poruka upravo je završila.
             * Ponovno nacrtaj Game 1 ekran.
             */
            if ((currentComboMessage == 0U) &&
                (lastComboMessage != 0U))
            {
                lastComboMessage = 0U;
                lastScore = currentScore;
                lastGame1Interval = currentGame1Interval;

                DrawGame1();
                return;
            }

            /*
             * Dok je COMBO poruka na ekranu,
             * ne crtamo score i interval preko nje.
             */
            if (currentComboMessage != 0U)
            {
                return;
            }

            if (currentScore != lastScore)
            {
                lastScore = currentScore;
                DrawGame1Score();
            }

            if (currentGame1Interval != lastGame1Interval)
            {
                lastGame1Interval = currentGame1Interval;
                DrawGame1Interval();
            }
        }

        if (GetSelectedGame() == 1U)
        {
            /*
             * COMBO promjena ima prioritet nad SCORE osvježavanjem.
             */
            if (currentGame2ComboRevision != lastGame2ComboRevision)
            {
                lastGame2ComboRevision =
                    currentGame2ComboRevision;

                /*
                 * COMBO upravo počinje.
                 */
                if (Game2_IsComboMessageActive() != 0U)
                {
                    /*
                     * Obriši cijelo područje gdje je bio score
                     * prije crtanja combo poruke.
                     */
                    ClearGame2ComboArea();

                    DrawGame2ComboBonus();

                    /*
                     * Važno:
                     * zapamti novi score kao već obrađen
                     * da se u sljedećem prolazu ne iscrta
                     * preko combo poruke.
                     */
                    lastScore = currentScore;

                    return;
                }

                /*
                 * COMBO upravo završava.
                 */
                ClearGame2ComboArea();

                DrawGame2Score();

                lastScore = currentScore;

                return;
            }

            /*
             * Dok COMBO traje, SCORE se ne smije crtati.
             */
            if (Game2_IsComboMessageActive() != 0U)
            {
                return;
            }

            /*
             * Normalno osvježavanje rezultata.
             */
            if (currentScore != lastScore)
            {
                lastScore = currentScore;

                DrawGame2Score();
            }
        }

        if (GetSelectedGame() == 2U)
        {
            /*
             * GAME 3:
             * osvježava se samo vrijednost SCORE-a.
             */
            if (currentScore != lastScore)
            {
                lastScore = currentScore;

                DrawGame3Score();
            }
        }

        return;
    }

    /*
     * Game Over ekran ponovno se crta ako se promijeni završni rezultat.
     */
    if (currentState == STATE_GAMEOVER)
    {
        /*
         * Prvi ulazak u GAME OVER.
         */
        if (currentState != lastState)
        {
            lastState = currentState;

            lastHighScoreRevision = Game1_GetHighScoreEntryRevision();

            lastGame2HighScoreRevision = currentGame2HighScoreRevision;

            lastGame3HighScoreRevision = currentGame3HighScoreRevision;

            lastCountdown = 255U;

            DrawGameOver();
            return;
        }

        /*
         * GAME 1:
         * promjena slova ili pozicije.
         */
        if ((GetSelectedGame() == 0U) &&
            (Game1_IsHighScoreEntryActive() != 0U))
        {
            uint32_t revision =
                Game1_GetHighScoreEntryRevision();

            if (revision != lastHighScoreRevision)
            {
                lastHighScoreRevision = revision;

                DrawHighScoreName();
            }

            return;
        }

        /*
         * GAME 2:
         * promjena slova ili pozicije.
         */
        if ((GetSelectedGame() == 1U) &&
            (Game2_IsHighScoreEntryActive() != 0U))
        {
            if (currentGame2HighScoreRevision !=
                lastGame2HighScoreRevision)
            {
                lastGame2HighScoreRevision =
                    currentGame2HighScoreRevision;

                DrawHighScoreName();
            }

            return;
        }

        /*
         * GAME 3:
         * promjena slova ili pozicije.
         */
        if ((GetSelectedGame() == 2U) &&
            (Game3_IsHighScoreEntryActive() != 0U))
        {
            if (currentGame3HighScoreRevision !=
                lastGame3HighScoreRevision)
            {
                lastGame3HighScoreRevision =
                    currentGame3HighScoreRevision;

                DrawHighScoreName();
            }

            return;
        }

        return;
    }

    if (currentState == STATE_SCORES)
    {
        uint8_t currentScoresGame = GetSelectedGame();

        if ((currentState == lastState) &&
            (currentScoresGame == lastScoresGame))
        {
            return;
        }

        lastState = currentState;
        lastScoresGame = currentScoresGame;
        lastCountdown = 255U;

        DrawScores(currentScoresGame);

        return;
    }

    /*
     * Za ostale ekrane dovoljne su promjena stanja
     * ili promjena odabrane stavke menija.
     */
    if ((currentState == lastState) &&
        (currentSelected == lastSelected))
    {
        return;
    }

    lastState = currentState;
    lastSelected = currentSelected;
    lastCountdown = 255U;

    switch (currentState)
    {
        case STATE_BOOT:
            DrawBootScreen();
            break;

        case STATE_MENU:
            DrawMenu(currentSelected);
            break;

        case STATE_GAME_READY:
            DrawGameReady();
            break;

        case STATE_GAME1:
            DrawGame1();
            break;

        case STATE_GAME2:
            DrawGame2();
            break;

        case STATE_GAME3:
            DrawGame3();
            break;

        default:
            break;
    }
}
