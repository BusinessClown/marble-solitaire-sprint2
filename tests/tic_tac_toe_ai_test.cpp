#include "tic_tac_toe_ai.h"

#include <optional>
#include <random>
#include <vector>

#include <gtest/gtest.h>

#include "tic_tac_toe_game.h"

namespace tictactoe {
namespace {

constexpr Difficulty kAllDifficulties[] = {
    Difficulty::kEasy,
    Difficulty::kNormal,
    Difficulty::kHard,
};

constexpr int kGamesPerMatchup = 50;

// Plays the listed moves, alternating players starting with X.
void PlayMoves(Game* game, const std::vector<Position>& moves) {
    for (const Position& move : moves) {
        ASSERT_TRUE(game->PlayMove(move.row, move.col));
    }
}

// Plays a full game in which X and O are each driven by the AI at the given
// difficulty, and returns how it ended.
GameState PlayOut(Difficulty x_level, Difficulty o_level,
                  std::mt19937* generator) {
    Game game;
    while (!game.is_over()) {
        const Difficulty level =
            game.current_player() == Player::kX ? x_level : o_level;
        const std::optional<Position> move = ChooseMove(game, level, generator);
        if (!move.has_value() || !game.PlayMove(move->row, move->col)) {
            ADD_FAILURE() << "AI produced no legal move";
            break;
        }
    }
    return game.state();
}

TEST(TicTacToeAiTest, EveryDifficultyReturnsALegalMove) {
    std::mt19937 generator(1);
    for (Difficulty level : kAllDifficulties) {
        SCOPED_TRACE(static_cast<int>(level));
        Game game;
        ASSERT_NO_FATAL_FAILURE(PlayMoves(&game, {{0, 0}, {1, 1}}));

        const std::optional<Position> move = ChooseMove(game, level, &generator);

        ASSERT_TRUE(move.has_value());
        EXPECT_EQ(game.CellAt(move->row, move->col), Cell::kEmpty);
    }
}

TEST(TicTacToeAiTest, ReturnsNothingOnceTheGameIsOver) {
    std::mt19937 generator(1);
    Game game;
    // X completes the top row.
    ASSERT_NO_FATAL_FAILURE(
        PlayMoves(&game, {{0, 0}, {1, 0}, {0, 1}, {1, 1}, {0, 2}}));
    ASSERT_TRUE(game.is_over());

    for (Difficulty level : kAllDifficulties) {
        SCOPED_TRACE(static_cast<int>(level));
        EXPECT_FALSE(ChooseMove(game, level, &generator).has_value());
    }
}

TEST(TicTacToeAiTest, EasyEventuallyPlaysEveryOpeningCell) {
    std::mt19937 generator(7);
    const Game game;
    bool seen[Game::kBoardSize][Game::kBoardSize] = {};

    for (int i = 0; i < 500; ++i) {
        const std::optional<Position> move =
            ChooseMove(game, Difficulty::kEasy, &generator);
        ASSERT_TRUE(move.has_value());
        seen[move->row][move->col] = true;
    }

    for (int row = 0; row < Game::kBoardSize; ++row) {
        for (int col = 0; col < Game::kBoardSize; ++col) {
            EXPECT_TRUE(seen[row][col]) << "row " << row << ", col " << col;
        }
    }
}

TEST(TicTacToeAiTest, NormalAndHardTakeAnImmediateWin) {
    std::mt19937 generator(1);
    // X has (0, 0) and (0, 1); O has (1, 0) and (1, 1). X wins at (0, 2), and
    // that must be preferred over blocking O at (1, 2).
    for (Difficulty level : {Difficulty::kNormal, Difficulty::kHard}) {
        SCOPED_TRACE(static_cast<int>(level));
        Game game;
        ASSERT_NO_FATAL_FAILURE(
            PlayMoves(&game, {{0, 0}, {1, 0}, {0, 1}, {1, 1}}));

        const std::optional<Position> move = ChooseMove(game, level, &generator);

        ASSERT_TRUE(move.has_value());
        EXPECT_EQ(move->row, 0);
        EXPECT_EQ(move->col, 2);
    }
}

TEST(TicTacToeAiTest, NormalAndHardBlockAnImmediateLoss) {
    std::mt19937 generator(1);
    // O has (1, 0) and (1, 1) and threatens (1, 2). X has no win of its own.
    for (Difficulty level : {Difficulty::kNormal, Difficulty::kHard}) {
        SCOPED_TRACE(static_cast<int>(level));
        Game game;
        ASSERT_NO_FATAL_FAILURE(
            PlayMoves(&game, {{0, 0}, {1, 0}, {2, 2}, {1, 1}}));

        const std::optional<Position> move = ChooseMove(game, level, &generator);

        ASSERT_TRUE(move.has_value());
        EXPECT_EQ(move->row, 1);
        EXPECT_EQ(move->col, 2);
    }
}

TEST(TicTacToeAiTest, HardNeverLosesAsX) {
    std::mt19937 generator(2024);
    for (Difficulty opponent : kAllDifficulties) {
        for (int i = 0; i < kGamesPerMatchup; ++i) {
            SCOPED_TRACE(static_cast<int>(opponent) * 1000 + i);
            EXPECT_NE(PlayOut(Difficulty::kHard, opponent, &generator),
                      GameState::kOWins);
        }
    }
}

TEST(TicTacToeAiTest, HardNeverLosesAsO) {
    std::mt19937 generator(2025);
    for (Difficulty opponent : kAllDifficulties) {
        for (int i = 0; i < kGamesPerMatchup; ++i) {
            SCOPED_TRACE(static_cast<int>(opponent) * 1000 + i);
            EXPECT_NE(PlayOut(opponent, Difficulty::kHard, &generator),
                      GameState::kXWins);
        }
    }
}

TEST(TicTacToeAiTest, TwoHardPlayersAlwaysDraw) {
    std::mt19937 generator(99);
    for (int i = 0; i < kGamesPerMatchup; ++i) {
        SCOPED_TRACE(i);
        EXPECT_EQ(PlayOut(Difficulty::kHard, Difficulty::kHard, &generator),
                  GameState::kDraw);
    }
}

TEST(TicTacToeAiTest, HardBeatsEasyMoreOftenThanItLoses) {
    std::mt19937 generator(5);
    int hard_wins = 0;
    for (int i = 0; i < kGamesPerMatchup; ++i) {
        if (PlayOut(Difficulty::kHard, Difficulty::kEasy, &generator) ==
            GameState::kXWins) {
            ++hard_wins;
        }
    }
    EXPECT_GT(hard_wins, kGamesPerMatchup / 2);
}

}  // namespace
}  // namespace tictactoe