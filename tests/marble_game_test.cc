#include "marble_game.h"

#include <algorithm>
#include <cstddef>
#include <random>
#include <vector>

#include <gtest/gtest.h>

namespace marble {
namespace {

constexpr BoardShape kAllShapes[] = {
    BoardShape::kFrench,   BoardShape::kJcWiegleb, BoardShape::kAsymmetrical,
    BoardShape::kEnglish,  BoardShape::kDiamond,   BoardShape::kTriangle,
};

Game MakeGame(BoardShape shape, BoardScale scale = BoardScale::kStandard) {
    Game game;
    game.Reset(shape, StartPosition::kStandard, scale);
    return game;
}

int CountSpots(const Game& game, Spot kind) {
    int count = 0;
    for (int row = 0; row < game.rows(); ++row) {
        for (int col = 0; col < game.cols(); ++col) {
            if (game.SpotAt(row, col) == kind) {
                ++count;
            }
        }
    }
    return count;
}

// Marbles plus empty holes: every spot that is part of the board.
int CountHoles(const Game& game) {
    return CountSpots(game, Spot::kMarble) + CountSpots(game, Spot::kEmpty);
}

bool ContainsMove(const std::vector<Move>& moves, const Move& wanted) {
    return std::any_of(moves.begin(), moves.end(), [&wanted](const Move& move) {
        return move.from == wanted.from && move.over == wanted.over &&
               move.to == wanted.to;
    });
}

std::vector<Move> AllMoves(const Game& game) {
    std::vector<Move> all;
    for (int row = 0; row < game.rows(); ++row) {
        for (int col = 0; col < game.cols(); ++col) {
            const std::vector<Move> moves = game.MovesFrom(row, col);
            all.insert(all.end(), moves.begin(), moves.end());
        }
    }
    return all;
}

Position OnlyMarble(const Game& game) {
    for (int row = 0; row < game.rows(); ++row) {
        for (int col = 0; col < game.cols(); ++col) {
            if (game.SpotAt(row, col) == Spot::kMarble) {
                return Position{row, col};
            }
        }
    }
    return Position{-1, -1};
}

// What the rules say a finished game should be called.
Outcome ExpectedOutcome(const Game& game) {
    switch (game.MarblesLeft()) {
    case 1:
        return OnlyMarble(game) == game.start_hole() ? Outcome::kPerfectGame
                                                     : Outcome::kWin;
    case 2:
        return Outcome::kTwoLeft;
    case 3:
        return Outcome::kThreeLeft;
    case 4:
        return Outcome::kFourLeft;
    default:
        return Outcome::kFiveOrMoreLeft;
    }
}

// Plays random legal jumps until none remain.
void PlayRandomlyUntilStuck(Game* game, std::mt19937* generator) {
    while (true) {
        const std::vector<Move> moves = AllMoves(*game);
        if (moves.empty()) {
            return;
        }
        std::uniform_int_distribution<std::size_t> pick(0, moves.size() - 1);
        ASSERT_TRUE(game->PlayMove(moves[pick(*generator)]));
    }
}

TEST(MarbleGameTest, DefaultGameIsStandardEnglishBoard) {
    const Game game;

    EXPECT_EQ(game.rows(), 7);
    EXPECT_EQ(game.cols(), 7);
    EXPECT_EQ(CountSpots(game, Spot::kMarble), 32);
    EXPECT_EQ(CountSpots(game, Spot::kEmpty), 1);
    EXPECT_EQ(game.captured_count(), 0);
}

TEST(MarbleGameTest, StandardBoardsHaveExpectedHoleCounts) {
    struct Case {
        BoardShape shape;
        int holes;
    };
    const Case cases[] = {
                           {BoardShape::kFrench, 37},   {BoardShape::kJcWiegleb, 45},
                           {BoardShape::kAsymmetrical, 39}, {BoardShape::kEnglish, 33},
                           {BoardShape::kDiamond, 41},  {BoardShape::kTriangle, 15},
                           };
    for (const Case& test_case : cases) {
        SCOPED_TRACE(static_cast<int>(test_case.shape));
        const Game game = MakeGame(test_case.shape);
        EXPECT_EQ(CountHoles(game), test_case.holes);
        EXPECT_EQ(CountSpots(game, Spot::kEmpty), 1);
        EXPECT_EQ(CountSpots(game, Spot::kMarble), test_case.holes - 1);
    }
}

TEST(MarbleGameTest, BoardsHaveExpectedDimensionsAtEveryScale) {
    struct Case {
        BoardShape shape;
        BoardScale scale;
        int rows;
        int cols;
    };
    const Case cases[] = {
                           {BoardShape::kFrench, BoardScale::kStandard, 7, 7},
                           {BoardShape::kFrench, BoardScale::kDouble, 13, 13},
                           {BoardShape::kFrench, BoardScale::kTriple, 19, 19},
                           {BoardShape::kEnglish, BoardScale::kStandard, 7, 7},
                           {BoardShape::kEnglish, BoardScale::kDouble, 13, 13},
                           {BoardShape::kEnglish, BoardScale::kTriple, 19, 19},
                           {BoardShape::kJcWiegleb, BoardScale::kStandard, 9, 9},
                           {BoardShape::kJcWiegleb, BoardScale::kDouble, 17, 17},
                           {BoardShape::kJcWiegleb, BoardScale::kTriple, 25, 25},
                           {BoardShape::kDiamond, BoardScale::kStandard, 9, 9},
                           {BoardShape::kDiamond, BoardScale::kDouble, 17, 17},
                           {BoardShape::kDiamond, BoardScale::kTriple, 25, 25},
                           {BoardShape::kAsymmetrical, BoardScale::kStandard, 8, 8},
                           {BoardShape::kAsymmetrical, BoardScale::kDouble, 15, 15},
                           {BoardShape::kAsymmetrical, BoardScale::kTriple, 22, 22},
                           {BoardShape::kTriangle, BoardScale::kStandard, 5, 9},
                           {BoardShape::kTriangle, BoardScale::kDouble, 9, 17},
                           {BoardShape::kTriangle, BoardScale::kTriple, 13, 25},
                           };
    const int max_size = Game::kMaxBoardSize;
    for (const Case& test_case : cases) {
        SCOPED_TRACE(static_cast<int>(test_case.shape) * 10 +
                     static_cast<int>(test_case.scale));
        const Game game = MakeGame(test_case.shape, test_case.scale);
        EXPECT_EQ(game.rows(), test_case.rows);
        EXPECT_EQ(game.cols(), test_case.cols);
        EXPECT_LE(game.rows(), max_size);
        EXPECT_LE(game.cols(), max_size);
    }
}

TEST(MarbleGameTest, DoubleScaledBoardsHaveExpectedHoleCounts) {
    struct Case {
        BoardShape shape;
        int holes;
    };
    const Case cases[] = {
                           {BoardShape::kFrench, 109},   {BoardShape::kEnglish, 69},
                           {BoardShape::kJcWiegleb, 93}, {BoardShape::kDiamond, 145},
                           {BoardShape::kTriangle, 45},
                           };
    for (const Case& test_case : cases) {
        SCOPED_TRACE(static_cast<int>(test_case.shape));
        const Game game = MakeGame(test_case.shape, BoardScale::kDouble);
        EXPECT_EQ(CountHoles(game), test_case.holes);
    }
}

TEST(MarbleGameTest, DoubleScaledFrenchBoardMatchesExpectedRowWidths) {
    const Game game = MakeGame(BoardShape::kFrench, BoardScale::kDouble);
    const int expected_widths[] = {3, 5, 7, 9, 11, 13, 13, 13, 11, 9, 7, 5, 3};

    ASSERT_EQ(game.rows(), 13);
    for (int row = 0; row < game.rows(); ++row) {
        SCOPED_TRACE(row);
        int width = 0;
        for (int col = 0; col < game.cols(); ++col) {
            if (game.SpotAt(row, col) != Spot::kOffBoard) {
                ++width;
            }
        }
        EXPECT_EQ(width, expected_widths[row]);
    }
}

TEST(MarbleGameTest, AsymmetricalBoardMatchesItsLayout) {
    const char* const expected[] = {
        "..###...", "..###...", "..###...", "########",
        "########", "########", "..###...", "..###...",
    };
    const Game game = MakeGame(BoardShape::kAsymmetrical);

    ASSERT_EQ(game.rows(), 8);
    ASSERT_EQ(game.cols(), 8);
    for (int row = 0; row < game.rows(); ++row) {
        for (int col = 0; col < game.cols(); ++col) {
            const bool is_hole = expected[row][col] == '#';
            EXPECT_EQ(game.SpotAt(row, col) != Spot::kOffBoard, is_hole)
                << "row " << row << ", col " << col;
        }
    }
}

TEST(MarbleGameTest, ScaledAsymmetricalBoardsHaveExpectedHoleCounts) {
    // The asymmetrical board is scaled by sampling the standard layout, so
    // each standard row and column is repeated two or three times.
    EXPECT_EQ(CountHoles(MakeGame(BoardShape::kAsymmetrical,
                                  BoardScale::kDouble)),
              144);
    EXPECT_EQ(CountHoles(MakeGame(BoardShape::kAsymmetrical,
                                  BoardScale::kTriple)),
              315);
}


TEST(MarbleGameTest, OnlyTheTriangleSpansTwoColumnsPerSpot) {
    for (BoardShape shape : kAllShapes) {
        SCOPED_TRACE(static_cast<int>(shape));
        const int expected = shape == BoardShape::kTriangle ? 2 : 1;
        EXPECT_EQ(MakeGame(shape).columns_per_spot(), expected);
    }
}

TEST(MarbleGameTest, TriangleSpotsAreStaggeredAndCentred) {
    const Game game = MakeGame(BoardShape::kTriangle);

    for (int row = 0; row < game.rows(); ++row) {
        int spots_in_row = 0;
        for (int col = 0; col < game.cols(); ++col) {
            if (game.SpotAt(row, col) != Spot::kOffBoard) {
                ++spots_in_row;
                EXPECT_EQ((row + col) % 2, 0);
            }
        }
        EXPECT_EQ(spots_in_row, row + 1);
    }
    EXPECT_NE(game.SpotAt(0, 4), Spot::kOffBoard);  // The apex.
}

TEST(MarbleGameTest, StandardStartEmptiesTheCenterMostHole) {
    struct Case {
        BoardShape shape;
        Position center;
    };
    const Case cases[] = {
                           {BoardShape::kFrench, {3, 3}},   {BoardShape::kEnglish, {3, 3}},
                           {BoardShape::kJcWiegleb, {4, 4}}, {BoardShape::kDiamond, {4, 4}},
                           {BoardShape::kTriangle, {2, 4}},
                           // The four center holes (3,3), (3,4), (4,3) and (4,4) are equally
                           // close to the middle of the 8x8 grid; the first one scanned wins.
                           {BoardShape::kAsymmetrical, {3, 3}},
                           };
    for (const Case& test_case : cases) {
        SCOPED_TRACE(static_cast<int>(test_case.shape));
        const Game game = MakeGame(test_case.shape);
        EXPECT_EQ(game.SpotAt(test_case.center.row, test_case.center.col),
                  Spot::kEmpty);
        EXPECT_TRUE(game.start_hole() == test_case.center);
    }
}

TEST(MarbleGameTest, RandomStartLeavesExactlyOneEmptyHole) {
    for (BoardShape shape : kAllShapes) {
        for (int attempt = 0; attempt < 10; ++attempt) {
            SCOPED_TRACE(static_cast<int>(shape) * 100 + attempt);
            Game game;
            game.Reset(shape, StartPosition::kRandom, BoardScale::kStandard);

            EXPECT_EQ(CountSpots(game, Spot::kEmpty), 1);
            EXPECT_EQ(game.SpotAt(game.start_hole().row, game.start_hole().col),
                      Spot::kEmpty);
        }
    }
}

TEST(MarbleGameTest, SpotAtOffGridIsOffBoard) {
    const Game game = MakeGame(BoardShape::kEnglish);

    EXPECT_EQ(game.SpotAt(-1, 0), Spot::kOffBoard);
    EXPECT_EQ(game.SpotAt(0, -1), Spot::kOffBoard);
    EXPECT_EQ(game.SpotAt(game.rows(), 0), Spot::kOffBoard);
    EXPECT_EQ(game.SpotAt(0, game.cols()), Spot::kOffBoard);
    EXPECT_EQ(game.SpotAt(0, 0), Spot::kOffBoard);  // English corner.
}

TEST(MarbleGameTest, EnglishBoardStartsWithTheFourCenterJumps) {
    const Game game = MakeGame(BoardShape::kEnglish);

    EXPECT_EQ(AllMoves(game).size(), 4u);
    EXPECT_TRUE(game.IsMovable(1, 3));
    EXPECT_TRUE(game.IsMovable(3, 1));
    EXPECT_TRUE(game.IsMovable(3, 5));
    EXPECT_TRUE(game.IsMovable(5, 3));

    const std::vector<Move> moves = game.MovesFrom(1, 3);
    ASSERT_EQ(moves.size(), 1u);
    EXPECT_TRUE(moves[0].from == (Position{1, 3}));
    EXPECT_TRUE(moves[0].over == (Position{2, 3}));
    EXPECT_TRUE(moves[0].to == (Position{3, 3}));
}

TEST(MarbleGameTest, SquareBoardsDoNotAllowDiagonalJumps) {
    // On the French board (1, 1) is diagonal from the empty center (3, 3) with
    // a marble between them, but a diagonal jump is not allowed.
    const Game game = MakeGame(BoardShape::kFrench);

    ASSERT_EQ(game.SpotAt(1, 1), Spot::kMarble);
    EXPECT_TRUE(game.MovesFrom(1, 1).empty());
    EXPECT_FALSE(game.IsMovable(1, 1));
}

TEST(MarbleGameTest, MovesFromReturnsNothingWithoutAJumpableMarble) {
    const Game game = MakeGame(BoardShape::kEnglish);

    EXPECT_TRUE(game.MovesFrom(3, 3).empty());    // Empty hole.
    EXPECT_TRUE(game.MovesFrom(0, 0).empty());    // Off the board.
    EXPECT_TRUE(game.MovesFrom(-1, -1).empty());  // Off the grid.
    EXPECT_TRUE(game.MovesFrom(2, 2).empty());    // Marble with no empty beyond.
}

TEST(MarbleGameTest, TriangleStartHasTwoMovableMarbles) {
    const Game game = MakeGame(BoardShape::kTriangle);

    EXPECT_EQ(AllMoves(game).size(), 2u);
    EXPECT_TRUE(game.IsMovable(4, 2));  // Diagonal jump over (3, 3).
    EXPECT_TRUE(game.IsMovable(4, 6));  // Diagonal jump over (3, 5).
}

TEST(MarbleGameTest, TriangleAllowsSidewaysJumps) {
    Game game = MakeGame(BoardShape::kTriangle);
    ASSERT_TRUE(game.PlayMove(Move{Position{4, 2}, Position{3, 3},
                                   Position{2, 4}}));

    // (4, 2) is now empty, so (4, 6) can jump along the bottom row over (4, 4).
    EXPECT_TRUE(ContainsMove(
        game.MovesFrom(4, 6),
        Move{Position{4, 6}, Position{4, 4}, Position{4, 2}}));
}

TEST(MarbleGameTest, PlayMoveCapturesTheJumpedMarble) {
    Game game = MakeGame(BoardShape::kEnglish);

    ASSERT_TRUE(game.PlayMove(Move{Position{1, 3}, Position{2, 3},
                                   Position{3, 3}}));

    EXPECT_EQ(game.SpotAt(1, 3), Spot::kEmpty);
    EXPECT_EQ(game.SpotAt(2, 3), Spot::kEmpty);
    EXPECT_EQ(game.SpotAt(3, 3), Spot::kMarble);
    EXPECT_EQ(game.captured_count(), 1);
    EXPECT_EQ(game.MarblesLeft(), 31);
}

TEST(MarbleGameTest, PlayMoveRejectsIllegalMoves) {
    Game game = MakeGame(BoardShape::kEnglish);

    // Starting from an empty hole.
    EXPECT_FALSE(game.PlayMove(Move{Position{3, 3}, Position{3, 4},
                                    Position{3, 5}}));
    // Landing somewhere other than the empty hole.
    EXPECT_FALSE(game.PlayMove(Move{Position{1, 3}, Position{2, 3},
                                    Position{3, 4}}));
    // Jumping a marble that is not adjacent to the mover.
    EXPECT_FALSE(game.PlayMove(Move{Position{3, 0}, Position{3, 2},
                                    Position{3, 3}}));
    // Off the grid.
    EXPECT_FALSE(game.PlayMove(Move{Position{-1, 0}, Position{0, 0},
                                    Position{1, 0}}));

    EXPECT_EQ(game.captured_count(), 0);
    EXPECT_EQ(game.MarblesLeft(), 32);
    EXPECT_EQ(game.SpotAt(3, 3), Spot::kEmpty);
}

TEST(MarbleGameTest, ResetClearsCapturedCount) {
    Game game = MakeGame(BoardShape::kEnglish);
    ASSERT_TRUE(game.PlayMove(Move{Position{1, 3}, Position{2, 3},
                                   Position{3, 3}}));
    ASSERT_EQ(game.captured_count(), 1);

    game.Reset(BoardShape::kEnglish, StartPosition::kStandard,
               BoardScale::kStandard);

    EXPECT_EQ(game.captured_count(), 0);
    EXPECT_EQ(game.MarblesLeft(), 32);
}

TEST(MarbleGameTest, HasMovesAtStartOfEveryBoard) {
    for (BoardShape shape : kAllShapes) {
        for (BoardScale scale : {BoardScale::kStandard, BoardScale::kDouble,
                                 BoardScale::kTriple}) {
            SCOPED_TRACE(static_cast<int>(shape) * 10 + static_cast<int>(scale));
            EXPECT_TRUE(MakeGame(shape, scale).HasMoves());
        }
    }
}

TEST(MarbleGameTest, RandomPlayoutsEndWithConsistentOutcomes) {
    std::mt19937 generator(12345);  // Fixed seed keeps the test repeatable.
    for (BoardShape shape : kAllShapes) {
        for (int playout = 0; playout < 25; ++playout) {
            SCOPED_TRACE(static_cast<int>(shape) * 100 + playout);
            Game game = MakeGame(shape);
            const int marbles_at_start = game.MarblesLeft();

            ASSERT_NO_FATAL_FAILURE(PlayRandomlyUntilStuck(&game, &generator));

            EXPECT_FALSE(game.HasMoves());
            EXPECT_GE(game.MarblesLeft(), 1);
            EXPECT_EQ(game.MarblesLeft() + game.captured_count(), marbles_at_start);
            EXPECT_EQ(game.FinalOutcome(), ExpectedOutcome(game));
        }
    }
}

}  // namespace
}  // namespace marble