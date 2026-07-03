#include <iostream>
#include <array>
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