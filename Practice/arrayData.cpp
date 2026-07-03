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