/*
Tic Tac Toe Game in C++.
*/

#include <iostream>
#include <cstdlib> // For system("cls") or system("clear")
using namespace std;    

void drawBoard(char board[3][3]);

int main() {
    // Initialize the Tic-Tac-Toe board with empty spaces
    char board[3][3] =
    {
        {' ', ' ', ' '},
        {' ', ' ', ' '},
        {' ', ' ', ' '}
    };

    char currentPlayer = 'X';
    int choice;
    bool gameWon = false;

    while (!gameWon) {
        // Display the board
        drawBoard(board);
        cout << "Current Board:(3x3)\n";
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                cout << board[i][j] << " ";
            }
            cout << endl;
        }

        // Get player input
        cout << "Player " << currentPlayer << ", enter your choice (1-9): ";
        cin >> choice;

        // Update the board based on player input
        int row = (choice - 1) / 3;
        int col = (choice - 1) % 3;
       
        if (board[row][col] != 'X' && board[row][col] != 'O') {
            
            board[row][col] = currentPlayer;
            
            // Check for a win condition here (not implemented)
            for (int i = 0; i < 3; i++) {
                //If same row values are equal then current player wins
                if (board[i][0] == currentPlayer && board[i][1] == currentPlayer && board[i][2] == currentPlayer)
                {
                    gameWon = true;
                    drawBoard(board);
                    cout << "Player " << currentPlayer << " wins!\n";
                }
                //If same column values are equal then current player wins
                else if (board[0][i] == currentPlayer && board[1][i] == currentPlayer && board[2][i] == currentPlayer)
                {
                    gameWon = true;
                    drawBoard(board);
                    cout << "Player " << currentPlayer << " wins!\n";
                }
                //If diagonal values are equal then current player wins
                else if (board[0][0] == currentPlayer && board[1][1] == currentPlayer && board[2][2] == currentPlayer)
                {
                    gameWon = true;
                    drawBoard(board);
                    cout << "Player " << currentPlayer << " wins!\n";
                }
                //If diagonal values are equal then current player wins
                else if (board[0][2] == currentPlayer && board[1][1] == currentPlayer && board[2][0] == currentPlayer)
                {
                    gameWon = true;
                    drawBoard(board);
                    cout << "Player " << currentPlayer << " wins!\n";
                }
                else
                {
                    gameWon = false;
                }
            }

            // Switch players
            currentPlayer = (currentPlayer == 'X') ? 'O' : 'X';

        } 
        else
        {
            cout << "Invalid move! Try again.\n";
        }
    }

    return 0;
}

// Function to draw the Tic-Tac-Toe board layout
void drawBoard(char board[3][3]) {
    // Clear the console screen to make the game look animated/smooth
    // Use "cls" for Windows, "clear" for Linux/macOS
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif

    cout << "\n      TIC-TAC-TOE\n\n";
    cout << "     Col 0   1   2 \n";
    cout << "          |   |   \n";
    cout << "Row 0   " << board[0][0] << " | " << board[0][1] << " | " << board[0][2] << " \n";
    cout << "       ___|___|___\n";
    cout << "          |   |   \n";
    cout << "Row 1   " << board[1][0] << " | " << board[1][1] << " | " << board[1][2] << " \n";
    cout << "       ___|___|___\n";
    cout << "          |   |   \n";
    cout << "Row 2   " << board[2][0] << " | " << board[2][1] << " | " << board[2][2] << " \n";
    cout << "          |   |   \n\n";
}