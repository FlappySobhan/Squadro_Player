#ifndef PIECE_HPP
#define PIECE_HPP

/**
 * @struct Piece
 * @brief Represents a single game piece on the Squadro board.
 *
 * This struct holds all the necessary information for a piece, including its
 * unique identifier, its current position on its track, which player it belongs to,
 * and whether it has completed its first crossing.
 */
struct Piece {
    int id;                 // Unique ID (0-4 for Player 0, 5-9 for Player 1)
    int position;           // Progress along its track (0=start, 6=far side)
    bool has_turned_around; // True if it has reached the far side
    int player;             // Owning player (0 or 1)
};

#endif // PIECE_HPP
