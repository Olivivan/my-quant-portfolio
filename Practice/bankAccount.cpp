/*
Bank Account Management System.
This program implements a simple bank account management system using C++.
*/

#include <iostream>
#include <iomanip>
#include <string>
#include "bankAccount.hpp"

using namespace std;

int main() {
    
    int choice;
    int accountNumber;
    
    double initialBalance = 0.0;
    double value = 0.0;

    string accountHolder;
    string password;

    // Assuming a single account holder for simplicity
    cout << "Account Holder: ";
    getline(cin, accountHolder); // Read the account holder's name

    //Account Number
    cout << "Account Number: ";
    cin >> accountNumber;

    //Password
    cout << "Password: "; 
    cin >> password;

    // Create a BankAccount object with the provided details
    BankAccount account(accountHolder, accountNumber, password, initialBalance);

    //Run System until user chooses to exit
    do {

        // Perform some operations on the account
        cout << "What would you like to do? (1: Check Balance, 2: Deposit, 3: Withdraw, 4: Withdraw All, 5: Exit): ";
        cin >> choice;

        switch(choice) {
            case 1:
                cout << fixed << setprecision(2);
                cout << "Current Balance: $" << account.getBalance() << endl;
                cout << defaultfloat;
                break;

            case 2:
                cout << "Enter deposit amount: ";
                cin >> value;

                account.deposit(value);
                break;

            case 3:
                cout << "Enter withdrawal amount: ";
                cin >> value;
                
                account.withdraw(value);
                break;
            
            case 4:
                value = account.getBalance();
                account.withdraw(value);
                cout << "All funds withdrawn." << endl;
                cout << "Current Balance: $" << account.getBalance() << endl;
                break;

            case 5:
                break;

            default:
                cout << "Invalid choice." << endl;
        }

    } while(choice != 5);

    return 0;
}

