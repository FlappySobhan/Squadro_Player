#include "MinimaxAI.hpp"
#include <limits>
#include <algorithm>
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <chrono>
#include <future>
#include <random> // Added for MCTS functionality
#include <map>    // Added for MCTS functionality

std::mutex print_mutex; // Prevents simultaneous printing from multiple threads

// A constant to control the influence of the MCTS score on the final combined score.
MinimaxAI::MinimaxAI(size_t num_threads) : pool(num_threads) {
    // The thread pool is initialized in the member initializer list
}

int MinimaxAI::findBestMove(const Board& board, const std::chrono::duration<double>& time_limit) {
    auto start_time = std::chrono::steady_clock::now();
    
    auto legalMoves = board.getLegalMoves();
    if (legalMoves.empty()) return -1;

    int best_move_overall = legalMoves[0];

    bool isMaximizing = (board.getCurrentPlayer() == 0);
    
    for (int depth = 1; depth < 30; ++depth) {
        auto elapsed = std::chrono::steady_clock::now() - start_time;
        if (elapsed > time_limit * 0.8) {
            std::cout << "Time limit approaching: "
                      << std::chrono::duration_cast<std::chrono::duration<double>>(elapsed).count()
                      << "s. Using best move from depth " << (depth - 1) << ".\n";
            break;
        }

        std::vector<std::future<int>> futures;
        for (int move : legalMoves) {
            // Enqueue the minimax search for each move as a task
            futures.emplace_back(
                pool.enqueue([this, &board, depth, isMaximizing, start_time, time_limit, move]() {
                    auto nextBoard = board.clone();
                    nextBoard->makeMove(move);
                    
                    int minimax_score = minimax(*nextBoard, depth - 1, !isMaximizing,
                                                std::numeric_limits<int>::min(),
                                                std::numeric_limits<int>::max(),
                                                start_time, time_limit);
                    
                    // The number of MCTS rollouts to perform.
                    // This can be adjusted based on performance needs.
                    const int NUM_MCTS_ROLLOUTS = 500;
                    int mcts_score = mctsRollout(*nextBoard, NUM_MCTS_ROLLOUTS);
                    
                    int combined_score = static_cast<int>(
                        0.7 * minimax_score +
                        0.3 * (mcts_score / NUM_MCTS_ROLLOUTS) * (60 - depth)
                    );
                    
                    return combined_score;
                })
            );
        }
        
        // Wait for all tasks to complete and collect their values
        std::vector<int> move_values;
        for(auto& future : futures) {
            move_values.push_back(future.get());
        }

        int best_move_this_depth = -1;
        int best_value = isMaximizing ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();

        for(size_t i = 0; i < legalMoves.size(); ++i) {
            if ((isMaximizing && move_values[i] > best_value) || (!isMaximizing && move_values[i] < best_value)) {
                best_value = move_values[i];
                best_move_this_depth = legalMoves[i];
            }
        }
        
        std::lock_guard<std::mutex> lock(print_mutex);
        std::cout << "Depth " << depth << " search completed. ";
        std::cout << "Best move is: " << best_move_this_depth
                  << " with value: " << best_value << "\n";
        
        best_move_overall = best_move_this_depth;
    }

    return best_move_overall;
}

int MinimaxAI::minimax(const Board& board, int depth, bool isMaximizingPlayer,
                       int alpha, int beta,
                       const std::chrono::steady_clock::time_point& start_time,
                       const std::chrono::duration<double>& time_limit)
{
    auto now = std::chrono::steady_clock::now();
    if (now - start_time > time_limit * 0.8) {
        return evaluateState(board); 
    }

    if (depth == 0 || board.isGameOver()) return evaluateState(board);

    auto legalMoves = board.getLegalMoves();
    if (legalMoves.empty()) return evaluateState(board);

    if (isMaximizingPlayer) {
        int maxEval = std::numeric_limits<int>::min();
        for (int move : legalMoves) {
            auto nextBoard = board.clone();
            nextBoard->makeMove(move);
            int eval = minimax(*nextBoard, depth - 1, false, alpha, beta, start_time, time_limit);
            maxEval = std::max(maxEval, eval);
            alpha = std::max(alpha, eval);
            if (beta <= alpha) break;
        }
        return maxEval;
    } else {
        int minEval = std::numeric_limits<int>::max();
        for (int move : legalMoves) {
            auto nextBoard = board.clone();
            nextBoard->makeMove(move);
            int eval = minimax(*nextBoard, depth - 1, true, alpha, beta, start_time, time_limit);
            minEval = std::min(minEval, eval);
            beta = std::min(beta, eval);
            if (beta <= alpha) break;
        }
        return minEval;
    }
}

// Function to perform a Monte Carlo Tree Search rollout.
int MinimaxAI::mctsRollout(const Board& board, int num_simulations) const {
    std::random_device rd;
    std::mt19937 gen(rd());
    
    int wins = 0;
    int losses = 0;

    for (int i = 0; i < num_simulations; ++i) {
        auto tempBoard = board.clone();
        
        while (!tempBoard->isGameOver()) {
            auto moves = tempBoard->getLegalMoves();
            if (moves.empty()) break;
            
            std::uniform_int_distribution<> distrib(0, moves.size() - 1);
            int random_move = moves[distrib(gen)];
            
            tempBoard->makeMove(random_move);
        }

        int winner = tempBoard->getWinner();
        if (winner == 0) { // MAX player wins
            wins++;
        } else if (winner == 1) { // MIN player wins
            losses++;
        }
    }
    
    return wins - losses;
}


// --- Evaluate State ---
int MinimaxAI::evaluateState(const Board& board) const {
    int winner = board.getWinner();
    if (winner == 0) return 1000;
    if (winner == 1) return -1000;

    int score = 0;
    const auto& pieces = board.getPieces();
    int p0_returned = 0, p1_returned = 0;

    for (const auto& p : pieces) {
        int piece_score = p.has_turned_around ? (6 - p.position) * 2 + 10 : p.position;
        if (p.player == 0) score += piece_score;
        else score -= piece_score;

        if (p.position == 0 && p.has_turned_around) {
            if (p.player == 0) p0_returned++;
            else p1_returned++;
        }
    }

    score += p0_returned * 30;
    score -= p1_returned * 30;

    return score;
}
