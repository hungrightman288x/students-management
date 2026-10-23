
#pragma once
#include <string>
#include "models/Date.h"

class Student {
    private:
        std::string StudentID;
        std::string FullName;
        Date DateOfBirth;
        std::string email;

    public:
        Student(std::string = "", std::string = "", int = 1, int = 1,
                int = 2000, std::string = "");

        std::string GetID() const;
        std::string GetFullName() const;
        std::string GetEmail() const;
        const Date& GetDateOfBirth() const;

        void SetEmail(std::string email);
        void SetFullName(std::string name);

        void DisplayInfo() const;
};
