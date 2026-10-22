
#pragma once
#include <string>
#include "models/Role.h"
using namespace std;

class Account {
    private:
        string UserName;
        string Password;
        Role UserRole;

    public:
        Account(string = "", string = "", Role = STUDENT);

        string GetUserName() const;
        Role GetUserRole() const;

        bool CheckPassword(const string& pass) const;
        void ChangePassword(const string& pass);

        void DisplayInfoAcc() const;
};
