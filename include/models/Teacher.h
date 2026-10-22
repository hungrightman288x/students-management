
#pragma once
#include <string>
#include "models/Date.h"
using namespace std;

class Teacher {
    private:
        string TeacherID;
        string FullName;
        Date DateOfBirth;
        string email;

    public:
        Teacher(string = "", string = "", int = 1, int = 1,
                int = 2000, string = "");

        string GetID() const;
        string GetFullName() const;
        string GetEmail() const;

        void SetFullName(const string& name);
        void SetEmail(const string& email);

        void DisplayInfo() const;
};
