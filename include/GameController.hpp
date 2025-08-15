#ifndef GAME_CONTROLLER_HPP
#define GAME_CONTROLLER_HPP

#include "Board.hpp"
#include "MinimaxAI.hpp"
#include <string>
#include <chrono>
#include <thread>
#include <mutex>

// Forward declaration for httplib
namespace httplib { class Server; }

/**
 * @class GameController
 * @brief Orchestrates the game flow using HTTP for communication.
 */
class GameController {
public:
    GameController(const std::string& host, int send_port, int receive_port, int ai_player_id);
    ~GameController();

    // The main game loop for the AI bot.
    void run();
    
private:
    Board board;
    MinimaxAI ai;
    
    std::string host_ip;
    int port_to_send;
    int port_to_receive;
    int ai_player;
    const std::chrono::duration<double> move_time_limit{10};
    bool ai_moved_this_turn = false;

    // HTTP Server to listen for opponent moves
    std::unique_ptr<httplib::Server> svr;
    std::thread server_thread;
    mutable std::mutex board_mutex; // Protects the board from simultaneous access

    void startListeningServer();
    void makeAndSendAIMove();
};

#endif // GAME_CONTROLLER_HPP
