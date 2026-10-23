
#include <iostream>
#include <stdexcept>
#include <cctype>
#include "models/Account.h"
using namespace std;

Account::Account(string user, string password, Role role, string profileID)
    : UserName(user), Password(password), UserRole(role), ProfileID(profileID) {
    SetProfile(role, profileID);
    if (!password.empty()) ChangePassword(password);
}

string Account::GetProfileID() const { return ProfileID; }
bool Account::HasPassword() const { return !Password.empty(); }

void Account::SetProfile(Role role, const string& profileID) {
    if (role != STUDENT && role != TEACHER && role != ADMIN)
        throw invalid_argument("Vai tro khong hop le");
    UserRole = role;
    ProfileID = profileID;
}

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
    bool hasText = false;
    for (unsigned char character : pass) {
        if (std::iscntrl(character)) throw invalid_argument("Mat khau chua ky tu dieu khien");
        if (!std::isspace(character)) hasText = true;
    }
    if (!hasText) {
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
    cout << "Profile ID: " << ProfileID << endl;
}
