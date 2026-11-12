#ifndef MARBLE_SOLITAIRE_SRC_TIC_TAC_TOE_WIDGET_H_
#define MARBLE_SOLITAIRE_SRC_TIC_TAC_TOE_WIDGET_H_

#include <random>

#include <QString>
#include <QWidget>

#include "tic_tac_toe_ai.h"
#include "tic_tac_toe_game.h"

class QMouseEvent;
class QPaintEvent;
class QTimer;

// Draws a tic-tac-toe board and turns mouse clicks into moves. Can also play
// O against the person at a chosen difficulty.
//
// All rules live in tictactoe::Game and all computer play in
// tictactoe::ChooseMove(); this class only paints the state, forwards clicks
// and reports status through StatusChanged(). Note that paintEvent() and
// mousePressEvent() keep Qt's lowerCamelCase spelling because they override
// virtual functions declared by QWidget.
class TicTacToeWidget : public QWidget {
    Q_OBJECT

public:
    explicit TicTacToeWidget(QWidget* parent = nullptr);

    // Chooses who the person plays against. With `vs_cpu` set, the computer
    // plays O at `difficulty` and the person plays X. Takes effect at the next
    // ResetBoard().
    void SetOpponent(bool vs_cpu, tictactoe::Difficulty difficulty);

    // Clears the board and starts a new game.
    void ResetBoard(bool x_goes_first);

signals:
    // Emitted whenever the status text changes, for example "X's turn",
    // "O wins!" or "It's a draw!".
    void StatusChanged(const QString& text);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    // Emits StatusChanged() with the game's current status text.
    void EmitStatus();

    // Returns true while the computer is the player to move.
    bool IsCpuTurn() const;

    // Starts the short pause before the computer moves, if it is its turn.
    void ScheduleCpuMove();

    // Makes the computer's move.
    void PlayCpuMove();

    tictactoe::Game game_;

    bool vs_cpu_ = false;
    tictactoe::Difficulty difficulty_ = tictactoe::Difficulty::kNormal;
    QTimer* cpu_timer_ = nullptr;
    std::mt19937 random_engine_{std::random_device{}()};
};

#endif  // MARBLE_SOLITAIRE_SRC_TIC_TAC_TOE_WIDGET_H_