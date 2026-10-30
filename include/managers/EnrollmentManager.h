
#pragma once
#include "models/Enrollment.h"
#include "models/Account.h"
#include "managers/StudentManager.h"
#include "managers/CourseSectionManager.h"
#include "managers/SubjectManager.h"
#include "managers/Manager.h"
#include <string>

using namespace std;

class EnrollmentManager : public Manager<Enrollment> {
private:
    string FileName;

    void LoadFromFile(
        const StudentManager&,
        const CourseSectionManager&
    );
    
    public:
    void SaveToFile() const;
    EnrollmentManager(
        const string&,
        const StudentManager&,
        const CourseSectionManager&
    );

    ~EnrollmentManager();

    void ViewStudentGrades(
        const Account& account,
        const StudentManager& students,
        const CourseSectionManager& sections,
        const SubjectManager& subjects
    ) const;
};
