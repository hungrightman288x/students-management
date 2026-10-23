
#include <iostream>
#include <stdexcept>
#include "models/Account.h"
using namespace std;

Account::Account(string user, string password, Role role)
    : UserName(user), Password(password), UserRole(role) {}

string Account::GetID() const {
    return this->UserName;
}

Role Account::GetUserRole() const {
    return this->UserRole;
}

bool Account::CheckPassword(const string& pass) const {
    return this->Password == pass;
}

void Account::ChangePassword(const string& pass) {
    if (pass.empty()) {
        throw invalid_argument("Mat khau khong duoc de trong");
    }

    this->Password = pass;
}

void Account::DisplayInfo() const {
    cout << "UserName: " << this->UserName << endl;

    cout << "Role: ";
    switch (this->UserRole) {
        case STUDENT:
            cout << "STUDENT";
            break;

        case TEACHER:
            cout << "TEACHER";
            break;

        case ADMIN:
            cout << "ADMIN";
            break;
    }

    cout << endl;
}
