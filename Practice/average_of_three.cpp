#include <iostream>

using namespace std;

int main() {
    double num1, num2, num3, average;

    cout << "Please enter a real number: ";
    cin >> num1;

    cout << "Now, enter another real number: ";
    cin >> num2;

    cout << "Now, enter the final  real number: ";
    cin >> num3;

    average = (num1 + num2 + num3) / 3;

    cout << "The average is: " << average << endl;

    return 0;
}