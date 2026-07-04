/*
VECTOR functionality in a simple C++ program.
This program demonstrates the sum of elements in a vector in C++.
*/

#include <iostream>
#include <vector>
using namespace std;

unsigned int sumVector(const vector<unsigned int>& myInts);

int main() {
    // Declare a vector of integers
    vector<unsigned int> myInts;
    int input;
    
    // Input elements into the vector
    cout << "Enter positive integers (enter -1 to stop): ";
    
    while (cin >> input && input != -1) {
        myInts.push_back(static_cast<unsigned int>(input));
    }

    // Display the sum of the elements in the vector
    cout << "The sum of the vector elements is: " << sumVector(myInts) << endl;

    return 0;
}

unsigned int sumVector(const vector<unsigned int>& myInts) {
    unsigned int sum = 0;
    for (std::size_t i = 0; i < myInts.size(); ++i) {
        sum += myInts[i]; // Calculate the sum of the elements
    }
    return sum;
}
