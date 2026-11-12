#include "tic_tac_toe_ai.h"

#include <algorithm>
#include <cstddef>
#include <optional>
#include <random>
#include <vector>

#include "tic_tac_toe_game.h"

namespace tictactoe {
namespace {

// Larger than any score Minimax() can return.
constexpr int kInfinity = 100;

// Score of a win at depth 0. Quicker wins score higher.
constexpr int kWinScore = 10;

std::vector<Position> EmptyCells(const Game& game) {
    std::vector<Position> cells;
    for (int row = 0; row < Game::kBoardSize; ++row) {
        for (int col = 0; col < Game::kBoardSize; ++col) {
            if (game.CellAt(row, col) == Cell::kEmpty) {
                cells.push_back(Position{row, col});
            }
        }
    }
    return cells;
}

Position PickRandom(const std::vector<Position>& cells,
                    std::mt19937* generator) {
    std::uniform_int_distribution<std::size_t> pick(0, cells.size() - 1);
    return cells[pick(*generator)];
}

Player OpponentOf(Player player) {
    return player == Player::kX ? Player::kO : Player::kX;
}

// Returns the first empty cell where `player` would complete a line.
std::optional<Position> FindCompletingCell(const Game& game,
                                           const std::vector<Position>& cells,
                                           Player player) {
    for (const Position& cell : cells) {
        if (game.WouldWin(player, cell.row, cell.col)) {
            return cell;
        }
    }
    return std::nullopt;
}

// Scores `game` from `cpu`'s point of view, assuming both sides play
// perfectly: positive if `cpu` wins, negative if it loses, 0 for a draw.
// `depth` is the number of moves played since the search started.
int Minimax(const Game& game, Player cpu, int depth) {
    switch (game.state()) {
    case GameState::kXWins:
        return cpu == Player::kX ? kWinScore - depth : depth - kWinScore;
    case GameState::kOWins:
        return cpu == Player::kO ? kWinScore - depth : depth - kWinScore;
    case GameState::kDraw:
        return 0;
    case GameState::kInProgress:
        break;
    }

    const bool is_cpu_turn = game.current_player() == cpu;
    int best = is_cpu_turn ? -kInfinity : kInfinity;
    for (const Position& cell : EmptyCells(game)) {
        Game next = game;
        next.PlayMove(cell.row, cell.col);
        const int score = Minimax(next, cpu, depth + 1);
        best = is_cpu_turn ? std::max(best, score) : std::min(best, score);
    }
    return best;
}

Position ChooseHardMove(const Game& game, const std::vector<Position>& cells,
                        std::mt19937* generator) {
    const Player cpu = game.current_player();
    std::vector<Position> best_cells;
    int best_score = -kInfinity;
    for (const Position& cell : cells) {
        Game next = game;
        next.PlayMove(cell.row, cell.col);
        const int score = Minimax(next, cpu, 1);
        if (score > best_score) {
            best_score = score;
            best_cells.clear();
        }
        if (score == best_score) {
            best_cells.push_back(cell);
        }
    }
    return PickRandom(best_cells, generator);
}

Position ChooseNormalMove(const Game& game,
                          const std::vector<Position>& cells,
                          std::mt19937* generator) {
    const Player cpu = game.current_player();
    if (const std::optional<Position> win =
        FindCompletingCell(game, cells, cpu)) {
        return *win;
    }
    if (const std::optional<Position> block =
        FindCompletingCell(game, cells, OpponentOf(cpu))) {
        return *block;
    }
    return PickRandom(cells, generator);
}

}  // namespace

std::optional<Position> ChooseMove(const Game& game, Difficulty difficulty,
                                   std::mt19937* generator) {
    if (game.is_over()) {
        return std::nullopt;
    }
    const std::vector<Position> cells = EmptyCells(game);
    if (cells.empty()) {
        return std::nullopt;
    }

    switch (difficulty) {
    case Difficulty::kEasy:
        return PickRandom(cells, generator);
    case Difficulty::kNormal:
        return ChooseNormalMove(game, cells, generator);
    case Difficulty::kHard:
        return ChooseHardMove(game, cells, generator);
    }
    return PickRandom(cells, generator);
}

}  // namespace tictactoe