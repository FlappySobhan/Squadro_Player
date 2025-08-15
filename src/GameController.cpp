#include "GameController.hpp"
#include <iostream>
#include <stdexcept>
#include "httplib.h"
#include "json.hpp"
using json = nlohmann::json;

GameController::GameController(const std::string& host, int send_port, int receive_port, int ai_player_id)
    : host_ip(host), 
      port_to_send(send_port), 
      port_to_receive(receive_port), 
      ai_player(ai_player_id), // This will be 1 or 2
      ai_moved_this_turn(false), // initialize the flag
      svr(std::make_unique<httplib::Server>())
{
    std::cout << "AI Bot initializing for Player " << ai_player << "...\n";
    std::cout << "Move time limit: " << move_time_limit.count() << " seconds.\n";
}

// Destructor
GameController::~GameController() {
    if (svr->is_running()) svr->stop();
    if (server_thread.joinable()) server_thread.join();
}

// Sets up and runs the HTTP server in a separate thread
void GameController::startListeningServer() {
    svr->Post("/", [this](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(board_mutex);

        int current_internal_player = board.getCurrentPlayer();
        int current_external_player = current_internal_player + 1;

        if (current_external_player == this->ai_player) {
            res.set_content("{\"status\": true, \"info\": \"ignored_as_not_opponent_turn\"}", "application/json");
            return;
        }

        try {
            json data = json::parse(req.body);
            int gui_move_id = data["move"];
            int server_player_id = data["player"];
            int internal_player_id = server_player_id - 1;
            int internal_piece_id = (gui_move_id - 1) + (internal_player_id * 5);

            std::cout << "Processing opponent's move for pawn " << gui_move_id 
                      << " (Internal ID: " << internal_piece_id << ")" << std::endl;

            board.makeMove(internal_piece_id);            
            ai_moved_this_turn = false; // reset flag for next turn
            res.set_content("{\"status\": true}", "application/json");

        } catch (const std::exception& e) {
            std::cerr << "Error handling opponent move: " << e.what() << '\n';
            res.set_content("{\"status\": false, \"error\": \"" + std::string(e.what()) + "\"}", "application/json");
            res.status = 400;
        }
    });

    server_thread = std::thread([this]() {
        std::cout << "HTTP server listening on http://0.0.0.0:" << port_to_receive << std::endl;
        if (!svr->listen("0.0.0.0", port_to_receive)) {
            std::cerr << "Fatal: Failed to start HTTP listening server.\n";
        }
    });
}

// Main game loop
void GameController::run() {
    startListeningServer();

    while (true) {
        bool is_my_turn = false;
        {
            std::lock_guard<std::mutex> lock(board_mutex);
            if (board.isGameOver()) break;
            is_my_turn = ((board.getCurrentPlayer() + 1) == this->ai_player);
        }

        if (is_my_turn && !ai_moved_this_turn) {
            makeAndSendAIMove();
            ai_moved_this_turn = true;
        } else if (!is_my_turn) {
            std::lock_guard<std::mutex> lock(board_mutex);
            ai_moved_this_turn = false;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    std::cout << "Game Over! Winner is Player " << board.getWinner() + 1 << std::endl;
}

// AI makes move and sends to GUI
void GameController::makeAndSendAIMove() {
    std::unique_ptr<Board> board_copy_ptr;
    {
        std::lock_guard<std::mutex> lock(board_mutex);
        if ((board.getCurrentPlayer() + 1) != ai_player) return;
        board_copy_ptr = board.clone();
    }

    std::cout << "AI is thinking...\n";
    int best_move_id = ai.findBestMove(*board_copy_ptr, move_time_limit);

    if (best_move_id == -1) {
        std::cerr << "AI could not find a legal move.\n";
        return;
    }

    int gui_move_to_send = (best_move_id % 5) + 1;
    std::cout << "AI chose pawn " << gui_move_to_send << " sent to GUI." << std::endl;

    json move_to_send_json;
    move_to_send_json["move"] = gui_move_to_send;
    std::string json_string = move_to_send_json.dump();

    httplib::Client cli(host_ip, port_to_send);
    cli.set_connection_timeout(5);

    auto res = cli.Post("/", json_string, "application/json");

    if (res && res->status == 200) {
        std::cout << "Server accepted move. Updating local board state.\n";
        std::lock_guard<std::mutex> lock(board_mutex);
        board.makeMove(best_move_id);
    } else {
        std::cerr << "Server rejected move." << std::endl;
        if(res) std::cerr << "Status code: " << res->status << std::endl;
        else std::cerr << "Error: " << httplib::to_string(res.error()) << std::endl;
    }
}
