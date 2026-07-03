#include <iostream>
#include <string>

using namespace std;

int main() {
    
    char whichPackage;
    int howManyJams, jamsIncluded, jamPrice, monthlyFee, totalCost;

    cout << "Which package do you own? A, B, or C? " << endl;
    cin >> whichPackage;

    cout << "How many jams, jellies, and preserves did you purchased this month? " << endl;
    cin >> howManyJams;

    // Determine the package details based on user input
    if (whichPackage == 'A' || whichPackage == 'a') {
        jamsIncluded = 2;
        jamPrice = 5;
        monthlyFee = 8;
    } else if (whichPackage == 'B' || whichPackage == 'b') {
        jamsIncluded = 4;
        jamPrice = 4;
        monthlyFee = 12;
    } else if (whichPackage == 'C' || whichPackage == 'c') {
        jamsIncluded = 6;
        jamPrice = 3;
        monthlyFee = 15;
    } else {
        cout << "Invalid package selection." << endl;
        return 1; // Exit the program with an error code
    }

   
    // Calculate the total cost based on the number of jams purchased
    if (howManyJams <= jamsIncluded) {
        totalCost = monthlyFee;
    }
    else if (howManyJams > jamsIncluded) {
        totalCost = (howManyJams - jamsIncluded) * jamPrice + monthlyFee;
    }
    

    cout << "Your total cost for this month is: $" << totalCost << endl;

    return 0;
}