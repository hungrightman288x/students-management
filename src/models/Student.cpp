
#include <iostream>
#include "models/Student.h"
using namespace std;

Student::Student(string id, string name, int day, int month,
                 int year, string email)
    : StudentID(id),
      FullName(name),
      DateOfBirth(day, month, year),
      email(email) {}

string Student::GetStudentId() const {
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
