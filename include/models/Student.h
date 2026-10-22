
#pragma once
#include <string>
#include "models/Date.h"
using namespace std;

class Student {
    private:
        string StudentID;
        string FullName;
        Date DateOfBirth;
        string email;

    public:
        Student(string = "", string = "", int = 1, int = 1,
                int = 2000, string = "");

        string GetID() const;
        string GetFullName() const;
        string GetEmail() const;

        void SetEmail(string email);
        void SetFullName(string name);

        void DisplayInfo() const;
};
