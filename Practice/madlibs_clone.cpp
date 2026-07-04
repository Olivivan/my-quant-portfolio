/*
STRINGS manipulation in a simple "Mad Libs" game implemented in C++.
This program is a simple "Mad Libs" game implemented in C++.
The user is prompted to input various words, such as adjectives, names, occupations,and hobbies
to make a funny story.
*/

#include <iostream>
#include <string>

using namespace std;

int main() {
    string adj1, adj2, adj3;
    string girl, boy, man;
    string occ1, occ2;
    string plc, item, hby;

    cout << "Please enter an adjective: ";
    getline(cin, adj1);

    cout << "Please enter girl's name: ";
    getline(cin, girl);

    cout << "Please enter another adjective: ";
    getline(cin, adj2);

    cout << "Please enter an occupation: ";
    getline(cin, occ1);

    cout << "Please enter name of a place: ";
    getline(cin, plc);

    cout << "Please enter an item of clothing in plural: ";
    getline(cin, item);

    cout << "Please enter a hobby: ";
    getline(cin, hby);

    cout << "Please enter another adjective: ";
    getline(cin, adj3);

    cout << "Please enter another occupation:";
    getline(cin, occ2);

    cout << "Please enter a boy's name: ";
    getline(cin, boy);

    cout << "Please enter a man's name: ";
    getline(cin, man);

    cout << "There once was a " << adj1 << " girl named " << girl << " who was a round "
         << occ1 << " in Kingdom of " << plc << "." << "She loved to wear " << item
         << " and " << hby << ". She wanted to marry " << adj2 << " " << occ2 << " named " << boy  
         <<" but her father, King " << man << " forbid her from seeing him." << endl;

    return 0;
}