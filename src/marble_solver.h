#ifndef MARBLE_SOLITAIRE_SRC_MARBLE_SOLVER_H_
#define MARBLE_SOLITAIRE_SRC_MARBLE_SOLVER_H_

#include <chrono>
#include <vector>

#include "marble_game.h"

namespace marble {

// How hard FindBestSolution() may search before it gives up.
struct SolverLimits {
    // Most positions the search may visit.
    int max_positions = 5000000;

    // Longest the search may run.
    std::chrono::milliseconds max_time{1500};
};

// A sequence of jumps that can be played, in order, from some position.
struct Solution {
    std::vector<Move> moves;

    // Marbles left on the board once `moves` have been played. A win is 1.
    int marbles_left = 0;
};

// Searches depth-first for a sequence of jumps that leaves one marble.
// Positions already seen are skipped, so each is explored at most once.
//
// If no winning sequence is found before `limits` run out (the position may
// be unsolvable, or the board too big to search), returns the sequence that
// left the fewest marbles. The result is always playable from `game`.
Solution FindBestSolution(const Game& game,
                          const SolverLimits& limits = SolverLimits());

}  // namespace marble

#endif  // MARBLE_SOLITAIRE_SRC_MARBLE_SOLVER_H_