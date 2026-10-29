#include "managers/EnrollmentManager.h"
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <cmath>
#include <set>
#include <unordered_map>
#include <utility>
using namespace std;
EnrollmentManager::EnrollmentManager(const string& FileName, const StudentManager& students, const CourseSectionManager& coursesections) : FileName(FileName){
    this->LoadFromFile(students, coursesections);
}
EnrollmentManager::~EnrollmentManager(){}
void EnrollmentManager::LoadFromFile(const StudentManager& students, const CourseSectionManager& coursesections){
    ifstream file(this->FileName);
    if(!file.is_open()){
        throw runtime_error("Khong the mo file " + this->FileName);
    }

    string line;
    set<pair<string, string>> StudentSections;
    unordered_map<string, int> SectionCounts;

    while (getline(file, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        if (line.empty()) {
            continue;
        }

        string EnrollmentID, StudentID, CourseSectionID;
        string DayText, MonthText, YearText;
        string AttendanceText, MidtermText, FinalText;
        stringstream row(line);

        if (!getline(row, EnrollmentID, '|') ||
            !getline(row, StudentID, '|') ||
            !getline(row, CourseSectionID, '|') ||
            !getline(row, DayText, '|') ||
            !getline(row, MonthText, '|') ||
            !getline(row, YearText, '|') ||
            !getline(row, AttendanceText, '|') ||
            !getline(row, MidtermText, '|') ||
            !getline(row, FinalText) ||
            FinalText.find('|') != string::npos) {
            throw invalid_argument("Dong dang ky hoc khong dung dinh dang");
        }

        size_t DayEnd = 0, MonthEnd = 0, YearEnd = 0;
        int Day = stoi(DayText, &DayEnd);
        int Month = stoi(MonthText, &MonthEnd);
        int Year = stoi(YearText, &YearEnd);

        size_t AttendanceEnd = 0, MidtermEnd = 0, FinalEnd = 0;
        float Attendance = stof(AttendanceText, &AttendanceEnd);
        float Midterm = stof(MidtermText, &MidtermEnd);
        float Final = stof(FinalText, &FinalEnd);

        if (DayEnd != DayText.size() ||
            MonthEnd != MonthText.size() ||
            YearEnd != YearText.size() ||
            AttendanceEnd != AttendanceText.size() ||
            MidtermEnd != MidtermText.size() ||
            FinalEnd != FinalText.size()) {
            throw invalid_argument("Ngay dang ky va diem khong duoc co ky tu thua");
        }

        if (!isfinite(Attendance) || !isfinite(Midterm) || !isfinite(Final)) {
            throw invalid_argument("Diem phai la so huu han");
        }

        if (students.FindByID(StudentID) == nullptr) {
            throw invalid_argument("Ma sinh vien khong ton tai: " + StudentID);
        }

        const CourseSection* section = coursesections.FindByID(CourseSectionID);
        if (section == nullptr) {
            throw invalid_argument("Ma lop hoc phan khong ton tai: " + CourseSectionID);
        }

        const pair<string, string> StudentSection(StudentID, CourseSectionID);
        if (StudentSections.find(StudentSection) != StudentSections.end()) {
            throw invalid_argument("Sinh vien da dang ky lop hoc phan: " +
                                   StudentID + " - " + CourseSectionID);
        }

        // Dem cac dang ky trong file, khong cong them vao si so da luu cua lop.
        if (SectionCounts[CourseSectionID] >= section->GetMaxStudents()) {
            throw invalid_argument("So dang ky vuot suc chua lop: " + CourseSectionID);
        }

        Enrollment* enrollment = new Enrollment(
            EnrollmentID, StudentID, CourseSectionID,
            Day, Month, Year, Attendance, Midterm, Final
        );

        if (!Add(enrollment)) {
            delete enrollment;
            throw invalid_argument("Trung ma dang ky hoc: " + EnrollmentID);
        }

        StudentSections.insert(StudentSection);
        ++SectionCounts[CourseSectionID];
    }

    if (file.bad()) {
        throw runtime_error("Loi khi doc file dang ky hoc");
    }
}
