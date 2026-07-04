/*
Class and Objects in C++.
*/

#include <iostream>
#include <string>
#include <vector>
using namespace std;

class Person{
    private:
        string firstName;
        string middleName;
        string lastName;

    public:
        // Constructor of initialization
        Person(string fName, string mName, string lName):
            firstName(fName), middleName(mName), lastName(lName){}

        // Setters
        void setFirstName(string fName){this->firstName = fName;}
        void setMiddleName(string mName){this->middleName = mName;}
        void setLastName(string lName){this->lastName = lName;}
        
        // Getters
        string getFirstName(){return firstName;}
        string getMiddleName(){return middleName;}
        string getLastName(){return lastName;}
        string getName(){return firstName + " " + middleName + " " + lastName;}


        // Method to display full name
        void displayFullName(){cout << "Full Name: " << firstName << " " << middleName << " " << lastName << endl;}

    };

class Employee: public Person{
    private:
        string department;
        string employeeId;
        double salary;

    public:
        // Constructor of initialization
        Employee(string fName, string mName, string lName, string depart, string eId, double sal)
            : Person(fName, mName, lName), department(depart), employeeId(eId), salary(sal) {}
        
        // Setters
        void setDepartment(string depart){this->department=depart;}
        void setEmployeeId(string eId){this->employeeId=eId;}
        void setSalary(double sal){this->salary=sal;}

        // Getters
        string getDepartment(){return department;}
        string getEmployeeId(){return employeeId;}
        double getSalary(){return salary;}

        // Method to display employee details
        void displayEmployeeDetails(){
            cout << "Employee ID: " << employeeId << endl;
            displayFullName();
            cout << "Department: " << department << endl;
            cout << "Salary: $" << salary << endl;
        }

};

int main(){

    char addChoice;
    bool exitChoice=false;
    string firstName, middleName, lastName, department, employeeId;
    double salary;

    vector<Person> persons;
    vector<Employee> employees;

    while(exitChoice==false){
        cout << "Initialize a person's details: " << endl;

        cout << "Enter First Name: ";
        getline(cin, firstName);
        
        cout << "Enter Middle Name: ";
        getline(cin, middleName);
        
        cout << "Enter Last Name: ";
        getline(cin, lastName);
        
        // Initialization and inclusion in persons vector.
        Person person(firstName, middleName, lastName);
        persons.push_back(person);

        // Determining if the current person is an Employee.
        cout << "Do you want to add an employee? (y/n): ";
        cin >> addChoice;
        cin.ignore(); // To ignore the newline character after reading choice

        if(addChoice == 'y' || addChoice == 'Y'){
            cout << "Enter Department: ";
            getline(cin, department);
            
            cout << "Enter Employee ID: ";
            getline(cin, employeeId);
            
            cout << "Enter Salary: ";
            cin >> salary;
            cin.ignore(); // To ignore the newline character after reading salary

            // Initializing and including to employees vector.
            Employee employee(firstName, middleName, lastName, department, employeeId, salary);
            employees.push_back(employee);
        }

        cout << "Do you want to add more persons:" << endl;
        cin >> addChoice;
        
        if (addChoice == 'n' || addChoice == 'N'){ exitChoice=true; }
        
    };
    
    cout << "Exiting system..." << endl;

    return 0;
}