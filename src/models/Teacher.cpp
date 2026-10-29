
#include <iostream>
#include <stdexcept>
#include "models/Teacher.h"
using namespace std;

Teacher::Teacher(string id, string name, int day, int month,
                 int year, string email)
    : TeacherID(id),
      FullName(name),
      DateOfBirth(day, month, year),
      email(email) {
        if(id.size() != 5 || id.substr(0, 2) != "GV"){
            throw invalid_argument("Ma gv phai co dang GVxxx");
        }
        for(int i = 2; i < 5; i++){
            if(id[i] < '0' || id[i] > '9'){
                throw invalid_argument("Ma gv phai co dang GVxxx");
            }
        }
      }

string Teacher::GetID() const {
    return this->TeacherID;
}

string Teacher::GetFullName() const {
    return this->FullName;
}

string Teacher::GetEmail() const {
    return this->email;
}

void Teacher::SetFullName(const string& name) {
    this->FullName = name;
}

void Teacher::SetEmail(const string& email) {
    this->email = email;
}

void Teacher::DisplayInfo() const {
    cout << "Teacher ID: " << this->TeacherID << endl;
    cout << "Full Name: " << this->FullName << endl;
    cout << "Date of Birth: " << this->DateOfBirth.to_string() << endl;
    cout << "Email: " << this->email << endl;
}
