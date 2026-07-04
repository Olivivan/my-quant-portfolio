/*
CLASSES:
This program demonstrates the use of classes in C++.
It allows the user to create a Pizza object, set its attributes (size and toppings),
*/

#include <iostream>
#include <string>
#include <vector>
using namespace std;

class Pizza {
public:
    string name;
    string size;
    vector<string> toppings;

    // Default constructor
    Pizza() = default;

    // Parameterized constructor
    Pizza(const string& name, const string& size, const vector<string>& toppings)
        : name(name), size(size), toppings(toppings) {}
};

int main() {
    // Create a Pizza object
    Pizza myPizza;

    cout << "Enter the name of the pizza: ";
    getline(cin, myPizza.name);

    cout << "Enter the size of the pizza: ";
    getline(cin, myPizza.size);
    
    
    return 0;
}