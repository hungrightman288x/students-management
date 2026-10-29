#pragma once
#include "models/Enrollment.h"
#include "managers/StudentManager.h"
#include "managers/CourseSectionManager.h"
#include "managers/Manager.h"
#include <string>
using namespace std;
class EnrollmentManager : public Manager<Enrollment>{
    private:
        string FileName;
        void LoadFromFile(const StudentManager&, const CourseSectionManager&);
    public:
        EnrollmentManager(const string&, const StudentManager&, const CourseSectionManager&);
        ~EnrollmentManager();
};