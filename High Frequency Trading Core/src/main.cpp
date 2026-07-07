#include <iostream>
#include "../lib/hft_core_lib/hft_core.hpp"
#include "../lib/hft_render_lib/hft_render.hpp"

enum class GameState { Initializing, Trading, Shutdown };

int main() {
    GameState currentState = GameState::Initializing;
    bool shouldClose = false;

    // This is the SINGLE main loop, as recommended to avoid "sub-loops" [9, 10]
    while (!shouldClose) {
        switch (currentState) {
            case GameState::Initializing:
                std::cout << "[HFT] Initializing Systems...\n";
                currentState = GameState::Trading;
                break;

            case GameState::Trading:
                // Redirect flow to specific library functions to keep main lean [11]
                // hft_core_update(); 
                // hft_render_metrics();
                break;

            case GameState::Shutdown:
                shouldClose = true;
                break;
        }
        
        // Temporary break for initial setup
        shouldClose = true;
    }

    return 0;
}