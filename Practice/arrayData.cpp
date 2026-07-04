/*
ARRAYS functionality in a simple C++ program.
This program demonstrates the use of arrays in C++.
It allows the user to input integers, stores them in an array,
and then displays the double of each element in the array.
*/

#include <iostream>
#include <array>

using namespace std;

int main() {
    // Declare an array of integers
    const int size = 5;
    array<int, size> myInts;

    // Store the elements of the array
    cout << "Array elements: ";
    for (int i = 0; i < size; i++)
    {
        cout << "Enter an integer: ";
        cin >> myInts[i];
    }

    // Display the double of the elements of the array
    cout << "The double of the array elements:"<< endl;
    for (int num : myInts)
    {
        cout << num * 2 << endl;
    }

    return 0;
}