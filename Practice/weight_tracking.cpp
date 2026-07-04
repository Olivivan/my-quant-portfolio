/*
PARALLEL VECTORS functionality in a simple C++ program.
This program demonstrates the use of parallel vectors in C++.
It allows the user to input names and weights of a fixed number of people,
stores them in two separate vectors, and then displays the names along with their corresponding weights.
*/

#include <iostream>
#include <vector>
#include <string>
using namespace std;

int main(){

    const size_t numPeople = 5;
    vector<size_t> weights;
    vector<string> names;

    for(size_t i = 0; i < numPeople; i++){
        string name;
        size_t weight;

        cout << "Enter the name of the person: ";
        getline(cin, name);
        names.push_back(name);

        cout << "Enter the weight of " << name << ": ";
        cin >> weight;
        weights.push_back(weight);
        cin.ignore(); // Clear the input buffer
    }

    cout << "\nWeight Tracking Results:\n";
    for(size_t i = 0; i < numPeople; i++){
        cout << names[i] << " weighs " << weights[i] << " pounds." << endl;
    }

    return 0;    
}