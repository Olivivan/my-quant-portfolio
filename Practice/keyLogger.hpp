/*
Keylooger Headers
*/
#ifndef KEYLOGGER_H
#define KEYLOGGER_H


#include <map>
#include <string>
using namespace std;

class KeyPair
{
    public:
        string VKName;
        string Name;

        //Constructors fo KeyPair
        KeyPair(const string &vk="", const string &name="")
            :VKName(vk), Name(name){}

};

class Key
{
    public:
        static map<int, KeyPair> KEYS;
};

#endif