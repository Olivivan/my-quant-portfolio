/*
FOR LOOP functionality in a simple "Odds and Evens" game implemented in C++.
*/
#include <iostream>

using namespace std;

int main() {
    
    for(int i = 0; i <= 50; i++) {
        if (i % 2 == 0) {
            cout << i << " is even." << endl;
        } else {
            cout << i << " is odd." << endl;
        }
    }

    return 0;
}