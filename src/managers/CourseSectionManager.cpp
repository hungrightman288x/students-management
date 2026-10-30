#include "managers/CourseSectionManager.h"
#include <sstream>
#include <fstream>
#include <stdexcept>
using namespace std;
CourseSectionManager::CourseSectionManager(const string& FileName, const SubjectManager& subjects, const TeacherManager& teachers) : FileName(FileName){
    this->LoadFromFile(subjects, teachers);
}
CourseSectionManager::~CourseSectionManager(){}
void CourseSectionManager::LoadFromFile(const SubjectManager& subjects, const TeacherManager& teachers){
    ifstream file(FileName);

    if(!file.is_open()){
        throw runtime_error("Khong the mo file " + FileName);
    }
    string line;
    while(getline(file, line)){
        if(!line.empty() && line.back() == '\r'){
            line.pop_back();
        }
        if(line.empty()){
            continue;
        }

        string CourseSectionID, SubjectID, TeacherID;
        string MaxStudentText, StudentNumberText;
        stringstream row(line);

        if( !getline(row, CourseSectionID, '|') || !getline(row, SubjectID, '|') || !getline(row, TeacherID, '|') ||
            !getline(row, MaxStudentText, '|') || !getline(row, StudentNumberText) || StudentNumberText.find('|') != string::npos)
        {
            throw invalid_argument("Dong lop hoc phan khong dung dinh dang");
        }

        size_t MaxStudentEnd = 0, StudentNumberEnd = 0;
        int MaxStudent = stoi(MaxStudentText, &MaxStudentEnd);
        int StudentNumber = stoi(StudentNumberText, &StudentNumberEnd);
        if (MaxStudentEnd != MaxStudentText.size() ||
            StudentNumberEnd != StudentNumberText.size()) {
            throw invalid_argument("Suc chua va si so khong duoc co ky tu thua");
        }

        if (subjects.FindByID(SubjectID) == nullptr) {
            throw invalid_argument("Ma mon hoc khong ton tai: " + SubjectID);
        }

        if (teachers.FindByID(TeacherID) == nullptr) {
            throw invalid_argument("Ma giang vien khong ton tai: " + TeacherID);
        }

        CourseSection* section = new CourseSection(CourseSectionID, SubjectID, TeacherID, MaxStudent, StudentNumber);

        if (!Add(section)){
            delete section;
            throw invalid_argument("Trung ma lop hoc phan: " + CourseSectionID);
        }
    }

    if (file.bad()) {
        throw runtime_error("Loi khi doc file lop hoc phan");
    }
}


void CourseSectionManager::SaveToFile() const {
    ofstream file(this->FileName);

    if (!file.is_open()) {
        throw runtime_error("Khong the mo file " + FileName);
    }

    for (const auto& pair : this->List) {
        const CourseSection* section = pair.second;

        file << section->GetID() << "|"
             << section->GetSubjectId() << "|"
             << section->GetTeacherId() << "|"
             << section->GetMaxStudents() << "|"
             << section->GetStudentNumber() << "\n";
    }

    file.flush();

    if (!file) {
        throw runtime_error("Loi khi ghi file lop hoc phan");
    }
}
