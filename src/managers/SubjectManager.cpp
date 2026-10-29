#include "managers/SubjectManager.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
using namespace std;
SubjectManager::SubjectManager(const string& FileName) : FileName(FileName){
    this->LoadFromFile();
}
SubjectManager::~SubjectManager(){}
void SubjectManager::LoadFromFile(){
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

        string SubjectID, SubjectName, CreditsText;
        stringstream row(line);

        if (!getline(row, SubjectID, '|') ||
            !getline(row, SubjectName, '|') ||
            !getline(row, CreditsText) ||
            CreditsText.find('|') != string::npos) {
            throw invalid_argument(
                "Dong mon hoc khong dung dinh dang"
            );
        }

        size_t CreditsEnd = 0;
        int Credits = stoi(CreditsText, &CreditsEnd);

        // stoi phai doc het chuoi, khong chap nhan "3abc" thanh 3.
        if (CreditsEnd != CreditsText.size()) {
            throw invalid_argument("So tin chi khong duoc co ky tu thua");
        }

        Subject* subject = new Subject(SubjectID, SubjectName, Credits);

        if (!Add(subject)) {
            delete subject;
            throw invalid_argument("Trung ma mon hoc: " + SubjectID);
        }
    }

    if (file.bad()) {
        throw runtime_error("Loi khi doc file mon hoc");
    }
}
