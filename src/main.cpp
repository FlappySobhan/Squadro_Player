#include "GameController.hpp"
#include <iostream>
#include <string>
#include <thread> // Include the thread library for parallel execution

int main(int argc, char* argv[]) {
    try {
        std::string server_host = "127.0.0.1";
        int send_port = 8081;
        int receive_port = 9081;
        int ai_player_id = 1;

        if (argc < 2) {
            std::cerr << "Usage: " << argv[0] << " --manual <server_ip> <send_port> <receive_port> <player_id>" << std::endl;
            std::cerr << "Or: " << argv[0] << " --demo" << std::endl;
            return 1;
        }

        std::string mode = argv[1];

        if (mode == "--manual") {
            if (argc != 6) {
                std::cerr << "Error: --manual mode requires 4 arguments: <server_ip> <send_port> <receive_port> <player_id>" << std::endl;
                return 1;
            }
            server_host = argv[2];
            send_port = std::stoi(argv[3]);
            receive_port = std::stoi(argv[4]);
            ai_player_id = std::stoi(argv[5]);

            if (ai_player_id != 1 && ai_player_id != 2) {
                std::cerr << "Error: Player ID must be 1 or 2." << std::endl;
                return 1;
            }

            std::cout << "Starting in manual mode for Player " << ai_player_id << "..." << std::endl;
            GameController controller(server_host, send_port, receive_port, ai_player_id);
            controller.run();

        } else if (mode == "--demo") {
            std::cout << "Starting in demo mode with two AI players..." << std::endl;
            
            // Player 1 configuration
            std::string player1_host = "127.0.0.1";
            int player1_send_port = 8081;
            int player1_receive_port = 9081;
            int player1_id = 1;

            // Player 2 configuration
            std::string player2_host = "127.0.0.1";
            int player2_send_port = 8082;
            int player2_receive_port = 9082;
            int player2_id = 2;

            // This is the new, multithreaded solution.
            // Create the controller objects on the heap so they live as long as the threads.
            auto controller1 = std::make_unique<GameController>(player1_host, player1_send_port, player1_receive_port, player1_id);
            auto controller2 = std::make_unique<GameController>(player2_host, player2_send_port, player2_receive_port, player2_id);
            
            // Start each controller's run method in a separate thread.
            std::cout << "Launching Player 1 and Player 2 threads..." << std::endl;
            std::thread player1_thread(&GameController::run, controller1.get());
            std::thread player2_thread(&GameController::run, controller2.get());

            // Wait for both threads to finish.
            player1_thread.join();
            player2_thread.join();

            std::cout << "Both AI players have finished their game." << std::endl;
        } else {
            std::cerr << "Error: Unknown mode. Use --manual or --demo." << std::endl;
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "An unhandled exception occurred: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
