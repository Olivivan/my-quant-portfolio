/*
VERCTORS and DO/WHILE functionality in a simple C++ program.
This program demonstrates the use of vectors in C++.
It allows the user to input positive integers, stores them in a vector,
and then displays the double of each element in the vector.
The program continues to prompt for input until a negative number is entered,
at which point it exits the loop and displays the results.
*/

#include <iostream>
#include <vector>

using namespace std;

int main() {
    // Declare a vector of integers
    vector<int> myInts;
    int positiveNum;

    do {
        cout << "Enter a positive number to add to the vector or a negative number to exit: ";
        cin >> positiveNum;
        
        if (positiveNum > 0) {
            // Store the elements of the vector
            myInts.push_back(positiveNum);
        }
    } while (positiveNum >= 0);

    // Display the double of the elements of the vector
    cout << "The double of the vector elements:"<< endl;
    for (int val : myInts)
    {
        cout << val * 2 << endl;
    }
    
    return 0;
}