#include "managers/StudentManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>
using namespace std;
StudentManager::StudentManager(const string& FileName) : FileName(FileName){
    LoadFromFile();
}
StudentManager::~StudentManager(){}
void StudentManager::LoadFromFile(){
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

        string StudentID, StudentName;
        string DayText, MonthText, YearText, Email;
        stringstream row(line);

        if (!getline(row, StudentID, '|') ||
            !getline(row, StudentName, '|') ||
            !getline(row, DayText, '|') ||
            !getline(row, MonthText, '|') ||
            !getline(row, YearText, '|') ||
            !getline(row, Email) ||
            Email.find('|') != string::npos) {
            throw invalid_argument(
                "Dong tai khoan khong dung dinh dang"
            );
        }

        size_t DayEnd = 0, MonthEnd = 0, YearEnd = 0;
        int Day = stoi(DayText, &DayEnd);
        int Month = stoi(MonthText, &MonthEnd);
        int Year = stoi(YearText, &YearEnd);

        // stoi phai doc het chuoi, khong chap nhan "20abc" thanh 20.
        if (DayEnd != DayText.size() ||
            MonthEnd != MonthText.size() ||
            YearEnd != YearText.size()) {
            throw invalid_argument("Ngay, thang, nam khong duoc co ky tu thua");
        }

        Student* student = new Student(StudentID, StudentName, Day, Month, Year, Email);

        if (!Add(student)) {
            delete student;
            throw invalid_argument("Trung username: " + StudentID);
        }
    }

    if (file.bad()) {
        throw runtime_error("Loi khi doc file tai khoan");
    }
}
