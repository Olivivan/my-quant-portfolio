/*
Functions construction and implementation.
*/

#include <iostream>
using namespace std;

// Function header declaration
int multiplyThree(int a, int b, int c);

int main(){
    
    int a, b, c;

    cout << "Enter a number to multiply: ";
    cin >> a;

    cout << "Enter a number to multiply: ";
    cin >> b;

    cout << "Enter a number to multiply: ";
    cin >> c;
    
    cout << "The product of " << a << ", " << b << ", and " << c << " is: " << multiplyThree(a, b, c) << endl;

    return 0;
}

//Function implementation
int multiplyThree(int a, int b, int c){
    return a * b * c;
}