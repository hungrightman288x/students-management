
#include <iostream>
#include "models/Student.h"
using namespace std;

Student::Student(string id, string name, int day, int month,
                 int year, string email)
    : StudentID(id),
      FullName(name),
      DateOfBirth(day, month, year),
      email(email) {
        if(id.size() != 5 || id.substr(0, 2) != "SV"){
            throw invalid_argument("Ma sinh vien co dang SVxxx");
        }
        for(int i = 2; i < 5; i++){
            if(id[i] < '0' || id[i] > '9'){
                throw invalid_argument("Ba ky tu cuoi phai la int");
            }
        }
      }

string Student::GetID() const {
    return this->StudentID;
}

string Student::GetFullName() const {
    return this->FullName;
}

string Student::GetEmail() const {
    return this->email;
}

void Student::SetEmail(string email) {
    this->email = email;
}

void Student::SetFullName(string name) {
    this->FullName = name;
}

void Student::DisplayInfo() const {
    cout << "Student ID: " << this->StudentID << endl;
    cout << "Full Name: " << this->FullName << endl;
    cout << "Date of Birth: " << this->DateOfBirth.to_string() << endl;
    cout << "Email: " << this->email << endl;
}

// Student.cpp
const Date& Student::GetDateOfBirth() const {
    return this->DateOfBirth;
}