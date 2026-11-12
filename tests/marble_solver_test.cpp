#include "marble_solver.h"

#include <chrono>

#include <gtest/gtest.h>

#include "marble_game.h"

namespace marble {
namespace {

constexpr BoardShape kAllShapes[] = {
    BoardShape::kFrench,   BoardShape::kJcWiegleb, BoardShape::kAsymmetrical,
    BoardShape::kEnglish,  BoardShape::kDiamond,   BoardShape::kTriangle,
};

constexpr BoardScale kAllScales[] = {
    BoardScale::kStandard,
    BoardScale::kDouble,
    BoardScale::kTriple,
};

Game MakeGame(BoardShape shape, BoardScale scale = BoardScale::kStandard) {
    Game game;
    game.Reset(shape, StartPosition::kStandard, scale);
    return game;
}

// Plays `solution` on a copy of `game`, checking that every move is legal and
// that the marble count matches what the solution claims.
void ExpectPlayable(Game game, const Solution& solution) {
    for (const Move& move : solution.moves) {
        ASSERT_TRUE(game.PlayMove(move));
    }
    EXPECT_EQ(game.MarblesLeft(), solution.marbles_left);
}

TEST(MarbleSolverTest, SolvesTheStandardEnglishBoard) {
    const Game game = MakeGame(BoardShape::kEnglish);

    const Solution solution = FindBestSolution(game);

    EXPECT_EQ(solution.marbles_left, 1);
    EXPECT_EQ(solution.moves.size(), 31u);  // One capture per move.
    ASSERT_NO_FATAL_FAILURE(ExpectPlayable(game, solution));
}

TEST(MarbleSolverTest, WinningSolutionLeavesAFinishedGame) {
    Game game = MakeGame(BoardShape::kEnglish);
    const Solution solution = FindBestSolution(game);
    ASSERT_EQ(solution.marbles_left, 1);

    for (const Move& move : solution.moves) {
        ASSERT_TRUE(game.PlayMove(move));
    }

    EXPECT_FALSE(game.HasMoves());
    EXPECT_EQ(game.MarblesLeft(), 1);
    EXPECT_EQ(game.captured_count(), static_cast<int>(solution.moves.size()));
}

TEST(MarbleSolverTest, FindsTheBestSolutionOnTheStandardTriangle) {
    const Game game = MakeGame(BoardShape::kTriangle);

    const Solution solution = FindBestSolution(game);

    EXPECT_LE(solution.marbles_left, 2);
    ASSERT_NO_FATAL_FAILURE(ExpectPlayable(game, solution));
}

TEST(MarbleSolverTest, TinyBudgetReturnsAnEmptyPlayableSolution) {
    const Game game = MakeGame(BoardShape::kEnglish);
    SolverLimits limits;
    limits.max_positions = 1;

    const Solution solution = FindBestSolution(game, limits);

    EXPECT_TRUE(solution.moves.empty());
    EXPECT_EQ(solution.marbles_left, game.MarblesLeft());
}

TEST(MarbleSolverTest, SolutionIsPlayableOnEveryBoardEvenWhenSearchIsCutShort) {
    SolverLimits limits;
    limits.max_positions = 2000;
    limits.max_time = std::chrono::milliseconds(5000);

    for (BoardShape shape : kAllShapes) {
        for (BoardScale scale : kAllScales) {
            SCOPED_TRACE(static_cast<int>(shape) * 10 + static_cast<int>(scale));
            const Game game = MakeGame(shape, scale);

            const Solution solution = FindBestSolution(game, limits);

            ASSERT_NO_FATAL_FAILURE(ExpectPlayable(game, solution));
            EXPECT_LT(solution.marbles_left, game.MarblesLeft());
        }
    }
}

TEST(MarbleSolverTest, DoesNotChangeTheGameItSearches) {
    const Game game = MakeGame(BoardShape::kEnglish);

    FindBestSolution(game);

    EXPECT_EQ(game.MarblesLeft(), 32);
    EXPECT_EQ(game.captured_count(), 0);
}

}  // namespace
}  // namespace marble