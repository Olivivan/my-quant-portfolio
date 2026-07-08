#include <iostream>
#include "../lib/hft_core_lib/hft_core.hpp"
#include "../lib/hft_render_lib/hft_render.hpp"

using namespace std;

// Professional State Machine as recommended
enum class AppState { Initializing, Trading, Error, Shutdown };

int main() {
    AppState currentState = AppState::Initializing;
    bool shouldClose = false;

    // THE SINGLE MAIN LOOP: Avoids sub-loops that destroy code flow
    while (!shouldClose) {
        switch (currentState) {
            case AppState::Initializing:
                // System setup logic
                currentState = AppState::Trading;
                break;

            case AppState::Trading:
                // Delegation: Logic handled by Core Lib, Display by Render Lib
                // hft_core_update();
                // hft_render_metrics();
                // Transition to shutdown when trading completes
                currentState = AppState::Shutdown;
                break;

            case AppState::Error:
                // Handle critical failures
                currentState = AppState::Shutdown;
                break;

            case AppState::Shutdown:
                shouldClose = true;
                break;
        }
    }

    return 0;
}