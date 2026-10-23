
#pragma once
#include <string>
#include <iosfwd>
#include "models/Role.h"

class Account {
    private:
        std::string UserName;
        std::string Password;
        Role UserRole;
        std::string ProfileID;
        friend void WriteRecord(std::ostream&, const Account&);

    public:
        Account(std::string = "", std::string = "", Role = STUDENT, std::string = "");

        std::string GetID() const;
        Role GetUserRole() const;
        std::string GetProfileID() const;
        bool HasPassword() const;
        void SetProfile(Role role, const std::string& profileID);

        bool CheckPassword(const std::string& pass) const;
        void ChangePassword(const std::string& pass);

        void DisplayInfo() const;
};
