
#pragma once
#include <string>
#include "models/Date.h"
#include "models/Grade.h"
using namespace std;

class Enrollment {
    private:
        string EnrollmentID;
        string StudentID;
        string CourseSectionID;
        Date EnrollmentDate;
        Grade Score;

    public:
        Enrollment(string = "", string = "", string = "",
                   int = 1, int = 1, int = 2000,
                   float = 0, float = 0, float = 0);

        string GetStudentId() const;
        string GetCourseSectionId() const;
        string GetID() const;

        const Date& GetEnrollmentDate() const;

        Grade& GetScore();
        const Grade& GetScore() const;

        void DisplayInfo() const;
};
