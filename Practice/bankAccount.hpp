/*
Class Bank Account headers file.
*/

#include <string>
#include <iostream>
using namespace std;

class BankAccount {
    private:
        string accountHolder;
        int accountNumber;
        string password;
        double balance;

    public:
        //Constructor to initialize the BankAccount object
        BankAccount(const string& holder, int number, const string& pass, double initialBalance)
            : accountHolder(holder), accountNumber(number), password(pass), balance(initialBalance)
        {
        }

        //Make a deposit to the account
        void deposit(double amount)
        {
            balance += amount;
        };

        //Make a withdrawal from the account
        void withdraw(double amount)
        {
            if (amount > 0 && amount <= balance)
            {
                balance -= amount;
            }
            else
            {
                cout << "Invalid withdrawal amount." << endl;
            };
        }
        
        //Getter Balance
        double getBalance()
        {
            return balance;
        };

        //Getter Account Holder
        string getAccountHolder() const
        {
            return accountHolder;
        };

        //Getter Account Number
        int getAccountNumber() const
        {
            return accountNumber;
        };
};
