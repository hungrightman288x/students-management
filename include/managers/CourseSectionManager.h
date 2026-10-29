#pragma once
#include "models/CourseSection.h"
#include "managers/SubjectManager.h"
#include "managers/TeacherManager.h"
#include "managers/Manager.h"
#include <string>
using namespace std;
class CourseSectionManager : public Manager<CourseSection>{
    private:
        string FileName;
        void LoadFromFile(const SubjectManager&, const TeacherManager&);
    public:
        CourseSectionManager(const string&, const SubjectManager&, const TeacherManager&);
        ~CourseSectionManager();
};