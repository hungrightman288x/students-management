#include "managers/TeacherManager.h"
#include <sstream>
#include <fstream>
#include <stdexcept>
using namespace std;
TeacherManager::TeacherManager(const string& FileName) : FileName(FileName){
    this->LoadFromFile();
}
TeacherManager::~TeacherManager(){}
void TeacherManager::LoadFromFile(){
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

        string TeacherID, FullName, DayText, MonthText, YearText, Email;
        stringstream row(line);

        if (!getline(row, TeacherID, '|') ||
            !getline(row, FullName, '|') ||
            !getline(row, DayText, '|') ||
            !getline(row, MonthText, '|') ||
            !getline(row, YearText, '|') ||
            !getline(row, Email) ||
            Email.find('|') != string::npos) {
            throw invalid_argument(
                "Dong giang vien khong dung dinh dang"
            );
        }

        size_t DayEnd = 0, MonthEnd = 0, YearEnd = 0;
        int Day = stoi(DayText, &DayEnd);
        int Month = stoi(MonthText, &MonthEnd);
        int Year = stoi(YearText, &YearEnd);

        if (DayEnd != DayText.size() ||
            MonthEnd != MonthText.size() ||
            YearEnd != YearText.size()) {
            throw invalid_argument("Ngay, thang, nam khong duoc co ky tu thua");
        }

        Teacher* teacher = new Teacher(TeacherID, FullName, Day, Month, Year, Email);

        if (!Add(teacher)) {
            delete teacher;
            throw invalid_argument("Trung ma giang vien: " + TeacherID);
        }
    }

    if (file.bad()) {
        throw runtime_error("Loi khi doc file giang vien");
    }
}
