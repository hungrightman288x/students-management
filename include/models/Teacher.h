
#pragma once
#include <string>
#include "models/Date.h"

class Teacher {
    private:
        std::string TeacherID;
        std::string FullName;
        Date DateOfBirth;
        std::string email;

    public:
        Teacher(std::string = "", std::string = "", int = 1, int = 1,
                int = 2000, std::string = "");

        std::string GetID() const;
        std::string GetFullName() const;
        std::string GetEmail() const;
        const Date& GetDateOfBirth() const;

        void SetFullName(const std::string& name);
        void SetEmail(const std::string& email);

        void DisplayInfo() const;
};
