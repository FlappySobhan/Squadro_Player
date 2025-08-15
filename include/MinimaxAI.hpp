#ifndef MINIMAX_AI_HPP
#define MINIMAX_AI_HPP

#include "Board.hpp"
#include "ThreadPool.hpp"
#include <chrono>

/**
 * @class MinimaxAI
 * @brief Implements the AI logic using Iterative Deepening Minimax with Alpha-Beta Pruning.
 */
class MinimaxAI {
public:
    MinimaxAI(size_t num_threads = 0);
    int findBestMove(const Board& board, const std::chrono::duration<double>& time_limit);

private:
    // The recursive Minimax function. Note the const Board& to avoid copying.
    int minimax(const Board& board, int depth, bool isMaximizingPlayer,
        int alpha, int beta,
        const std::chrono::steady_clock::time_point&,
        const std::chrono::duration<double>&);
    ThreadPool pool; // Member variable for the thread pool


    int mctsRollout(const Board& board, int num_simulations) const;
    
    int evaluateState(const Board& board) const;
};

#endif // MINIMAX_AI_HPP
