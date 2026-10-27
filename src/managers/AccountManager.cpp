#include "managers/AccountManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;
AccountManager::AccountManager(const string& FileName) : FileName(FileName){
    LoadFromFile();
}
AccountManager::~AccountManager(){}
void AccountManager::LoadFromFile() {
    ifstream file(FileName);

    if (!file.is_open()) {
        throw runtime_error("Khong mo duoc file: " + FileName);
    }

    string line;

    while (getline(file, line)) {
        // Bo ky tu xuong dong CR neu co.
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        if (line.empty()) {
            continue;
        }

        string user, password, roleText;
        stringstream row(line);

        if (!getline(row, user, '|') ||
            !getline(row, password, '|') ||
            !getline(row, roleText) ||
            roleText.find('|') != string::npos) {
            throw invalid_argument(
                "Dong tai khoan khong dung dinh dang"
            );
        }

        Role role;

        if (roleText == "STUDENT") {
            role = STUDENT;
        } else if (roleText == "TEACHER") {
            role = TEACHER;
        } else if (roleText == "ADMIN") {
            role = ADMIN;
        } else {
            throw invalid_argument("Role trong file khong hop le");
        }

        Account* account = new Account(user, password, role);

        if (!Add(account)) {
            delete account;
            throw invalid_argument("Trung username: " + user);
        }
    }

    if (file.bad()) {
        throw runtime_error("Loi khi doc file tai khoan");
    }
}