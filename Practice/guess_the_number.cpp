#include <iostream>

using namespace std;

int main(){
    start:
    int guess;
    int guessCount = 0;
    bool guessedNumber = false;
    
    // Generate a random number between 1 and 100
    int computerNumber = rand() % 100 + 1;
    
    while(guessedNumber==false){
        cout << "Enter a number between 1 and 100: ";
        cin >> guess;
        guessCount++;

        if(guess < 1 || guess > 100){
            cout << "Wasted guess. Enter a number between 1 and 100." << endl;
        }
        else if(guess > computerNumber){
            cout << "Too high! Try again." << endl;
        }
        else if(guess < computerNumber){
            cout << "Too low! Try again." << endl;
        }
        else{
            guessedNumber = true; // The user guessed the correct number
            cout << "Congratulations! You guessed the number in " << guessCount << " guesses!"
                 << " Thanks for playing!" << endl;
        }
    }
   
    char y_or_n;

    cout << "Want to play again? Y/N" << endl;
    cin >> y_or_n;
    
    if(y_or_n == 'Y' || y_or_n == 'y'){
        guessedNumber = false;
        goto start; // Restart the game
    }
    else{
        cout << "Thanks for playing! Exiting..." << endl;
    }

    return 0;
}