#include "marble_solver.h"

#include <chrono>
#include <cstdint>
#include <unordered_set>
#include <vector>

namespace marble {
namespace {

// FNV-1a over every spot of the grid. A collision can only make the search
// skip a position it should have explored; it can never produce an illegal
// move, because every move comes from Game::AllMoves().
std::uint64_t HashPosition(const Game& game) {
    constexpr std::uint64_t kOffsetBasis = 14695981039346656037ULL;
    constexpr std::uint64_t kPrime = 1099511628211ULL;

    std::uint64_t hash = kOffsetBasis;
    for (int row = 0; row < game.rows(); ++row) {
        for (int col = 0; col < game.cols(); ++col) {
            hash ^= static_cast<std::uint64_t>(game.SpotAt(row, col));
            hash *= kPrime;
        }
    }
    return hash;
}

// One depth-first search. Use it once.
class Search {
public:
    explicit Search(const SolverLimits& limits)
        : max_positions_(limits.max_positions),
        deadline_(std::chrono::steady_clock::now() + limits.max_time) {}

    Solution Run(const Game& start) {
        best_.marbles_left = start.MarblesLeft();
        seen_.insert(HashPosition(start));
        Explore(start);
        return best_;
    }

private:
    // Returns true when the search should stop: a win was found or a limit
    // was reached.
    bool Explore(const Game& game) {
        if (++visited_ > max_positions_ ||
            std::chrono::steady_clock::now() > deadline_) {
            return true;
        }

        const std::vector<Move> moves = game.AllMoves();
        if (moves.empty()) {
            const int marbles_left = game.MarblesLeft();
            if (marbles_left < best_.marbles_left) {
                best_.moves = path_;
                best_.marbles_left = marbles_left;
            }
            return marbles_left == 1;
        }

        for (const Move& move : moves) {
            Game next = game;
            next.PlayMove(move);
            if (!seen_.insert(HashPosition(next)).second) {
                continue;
            }
            path_.push_back(move);
            if (Explore(next)) {
                return true;
            }
            path_.pop_back();
        }
        return false;
    }

    const int max_positions_;
    const std::chrono::steady_clock::time_point deadline_;
    int visited_ = 0;
    std::unordered_set<std::uint64_t> seen_;
    std::vector<Move> path_;
    Solution best_;
};

}  // namespace

Solution FindBestSolution(const Game& game, const SolverLimits& limits) {
    Search search(limits);
    return search.Run(game);
}

}  // namespace marble