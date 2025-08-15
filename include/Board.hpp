#ifndef BOARD_HPP
#define BOARD_HPP

#include "Piece.hpp"
#include <vector>
#include <string>
#include <memory>

class Board {
public:
    Board();
    std::unique_ptr<Board> clone() const;

    // Game State Queries
    bool isGameOver() const;
    int getWinner() const;
    int getCurrentPlayer() const;
    std::vector<int> getLegalMoves() const;

    // --- NEW GETTER FOR AI EVALUATION ---
    const std::vector<Piece>& getPieces() const;

    // Game Actions
    void makeMove(int pieceId);

private:
    std::vector<Piece> pieces;
    int currentPlayer;

    const std::vector<int> speeds_h = {1, 3, 2, 3, 1};
    const std::vector<int> speeds_v = {3, 1, 2, 1, 3};

    int getPieceSpeed(const Piece& piece) const;
    void switchPlayer();
};

#endif // BOARD_HPP
