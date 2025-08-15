#include "Board.hpp"
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <map>

Board::Board() : currentPlayer(0) {
    // Player 0 (Horizontal, IDs 0-4)
    for (int i = 0; i < 5; ++i) {
        pieces.push_back({i, 0, false, 0});
    }
    // Player 1 (Vertical, IDs 5-9)
    for (int i = 0; i < 5; ++i) {
        pieces.push_back({i + 5, 0, false, 1});
    }
}

std::unique_ptr<Board> Board::clone() const {
    auto newBoard = std::make_unique<Board>();
    newBoard->pieces = this->pieces;
    newBoard->currentPlayer = this->currentPlayer;
    return newBoard;
}

bool Board::isGameOver() const {
    return getWinner() != -1;
}

int Board::getWinner() const {
    int returned_h = 0;
    int returned_v = 0;
    for (const auto& piece : pieces) {
        if (piece.player == 0 && piece.position == 0 && piece.has_turned_around) {
            returned_h++;
        } else if (piece.player == 1 && piece.position == 0 && piece.has_turned_around) {
            returned_v++;
        }
    }
    if (returned_h >= 4) return 0;
    if (returned_v >= 4) return 1;
    return -1;
}

int Board::getCurrentPlayer() const {
    return currentPlayer;
}

const std::vector<Piece>& Board::getPieces() const {
    return pieces;
}

std::vector<int> Board::getLegalMoves() const {
    std::vector<int> legal_moves;
    int start_id = (currentPlayer == 0) ? 0 : 5;
    for (int i = 0; i < 5; ++i) {
        const auto& piece = pieces[start_id + i];
        if (!(piece.position == 0 && piece.has_turned_around)) {
            legal_moves.push_back(piece.id);
        }
    }
    return legal_moves;
}

void Board::makeMove(int pieceId) {
    if (pieceId < 0 || pieceId >= 10) {
        throw std::out_of_range("Invalid piece ID in makeMove");
    }

    Piece& moving_piece = pieces[pieceId];
    if (moving_piece.player != currentPlayer) {
         throw std::logic_error("Attempted to move opponent's piece.");
    }

    int speed = getPieceSpeed(moving_piece);
    int direction = moving_piece.has_turned_around ? -1 : 1;
    int current_pos = moving_piece.position;
    bool jump_occurred_on_move = false;

    // --- FINAL, DEFINITIVE REWRITE OF JUMP LOGIC ---

    // 1. Simulate the initial move step by step
    for (int i = 0; i < speed; ++i) {
        int next_pos = current_pos + direction;

        // Check for an opponent at the next step's intersection
        Piece* opponent_to_jump = nullptr;
        for (auto& opponent : pieces) {
            if (opponent.player == moving_piece.player) continue;

            int moving_track = (moving_piece.id % 5) + 1;
            int opponent_track = (opponent.id % 5) + 1;
            int opponent_pos = opponent.position;

            if (moving_piece.player == 0 && opponent_pos == moving_track && opponent_track == next_pos) {
                opponent_to_jump = &opponent;
                break;
            } else if (moving_piece.player == 1 && opponent_pos == moving_track && opponent_track == next_pos) {
                opponent_to_jump = &opponent;
                break;
            }
        }

        if (opponent_to_jump) {
            // First jump happens, move ends here.
            opponent_to_jump->position = opponent_to_jump->has_turned_around ? 6 : 0;
            current_pos = next_pos;
            jump_occurred_on_move = true;
            break; // Stop the initial move
        } else {
            // No jump, just move one step.
            current_pos = next_pos;
        }
    }

    // 2. If a jump occurred on the initial move, land one space after.
    if (jump_occurred_on_move) {
        current_pos += direction;
    }
    
    // 3. Handle chain reactions from the landing spot
    while (true) {
        Piece* opponent_in_landing_spot = nullptr;
        for (auto& opponent : pieces) {
            if (opponent.player == moving_piece.player) continue;

            int moving_track = (moving_piece.id % 5) + 1;
            int opponent_track = (opponent.id % 5) + 1;
            int opponent_pos = opponent.position;

            if (moving_piece.player == 0 && opponent_pos == moving_track && opponent_track == current_pos) {
                opponent_in_landing_spot = &opponent;
                break;
            } else if (moving_piece.player == 1 && opponent_pos == moving_track && opponent_track == current_pos) {
                opponent_in_landing_spot = &opponent;
                break;
            }
        }

        if (opponent_in_landing_spot) {
            // Chain jump! Reset this opponent and move one step further.
            opponent_in_landing_spot->position = opponent_in_landing_spot->has_turned_around ? 6 : 0;
            current_pos += direction;
        } else {
            // No opponent in the landing spot, the chain is over.
            break;
        }
    }

    moving_piece.position = current_pos;
    
    // --- END FINAL REWRITE ---

    // Clamp position and handle turnarounds
    if (!moving_piece.has_turned_around && moving_piece.position >= 6) {
        moving_piece.position = 6;
        moving_piece.has_turned_around = true;
    } else if (moving_piece.has_turned_around && moving_piece.position <= 0) {
        moving_piece.position = 0;
    }
    
    switchPlayer();
}

int Board::getPieceSpeed(const Piece& piece) const {
    int base_speed;
    if (piece.player == 0) {
        base_speed = speeds_h[piece.id % 5];
    } else {
        base_speed = speeds_v[piece.id % 5];
    }

    if (piece.has_turned_around) {
        if (base_speed == 1) return 3;
        if (base_speed == 3) return 1;
    }
    return base_speed;
}

void Board::switchPlayer() {
    currentPlayer = 1 - currentPlayer;
}
