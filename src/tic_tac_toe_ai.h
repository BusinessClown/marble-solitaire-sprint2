#ifndef MARBLE_SOLITAIRE_SRC_TIC_TAC_TOE_AI_H_
#define MARBLE_SOLITAIRE_SRC_TIC_TAC_TOE_AI_H_

#include <optional>
#include <random>

#include "tic_tac_toe_game.h"

namespace tictactoe {

// How well the computer plays. The values must match the ids given to the
// tiles in kDifficultyOptions in main_window.cc.
enum class Difficulty {
    // Any empty cell, chosen at random.
    kEasy = 0,
    // Takes a winning move, otherwise blocks the opponent's winning move,
    // otherwise picks at random.
    kNormal = 1,
    // Plays perfectly (minimax), so it never loses. Equally good moves are
    // chosen at random so games vary.
    kHard = 2,
};

// Chooses a move for the player whose turn it is in `game`. Returns
// std::nullopt if the game is over. `generator` must not be null.
std::optional<Position> ChooseMove(const Game& game, Difficulty difficulty,
                                   std::mt19937* generator);

}  // namespace tictactoe

#endif  // MARBLE_SOLITAIRE_SRC_TIC_TAC_TOE_AI_H_