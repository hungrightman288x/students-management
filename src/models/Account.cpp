
#include <iostream>
#include <stdexcept>
#include "models/Account.h"
using namespace std;

Account::Account(string user, string password, Role role)
    : UserName(user), Password(password), UserRole(role) {
        switch (role) {
        case STUDENT:
            if (user.size() != 5 || user.substr(0, 2) != "SV") {
                throw invalid_argument(
                    "Username sinh vien phai co dang SVxxx"
                );
            }
            break;

        case TEACHER:
            if (user.size() != 5 || user.substr(0, 2) != "GV") {
                throw invalid_argument(
                    "Username giao vien phai co dang GVxxx"
                );
            }
            break;

        case ADMIN:
            if (user != "admin") {
                throw invalid_argument(
                    "Username quan tri vien phai la admin"
                );
            }
            break;

        default:
            throw invalid_argument("Role khong hop le");
    }

    // Sinh vien va giao vien phai co 3 chu so cuoi.
    if (role == STUDENT || role == TEACHER) {
        for (int i = 2; i < 5; ++i) {
            if (user[i] < '0' || user[i] > '9') {
                throw invalid_argument(
                    "Ba ky tu cuoi username phai la chu so"
                );
            }
        }
    }
    if(password == ""){
        throw invalid_argument("Mat khau khong duoc de trong");
    }
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
    cout << "\nPass: " << this->Password;
    cout << endl;
}
