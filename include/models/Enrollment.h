
#pragma once
#include <string>
#include "models/Date.h"
#include "models/Grade.h"

class Enrollment {
    private:
        std::string EnrollmentID;
        std::string StudentID;
        std::string CourseSectionID;
        Date EnrollmentDate;
        Grade Score;

    public:
        Enrollment(std::string = "", std::string = "", std::string = "",
                   int = 1, int = 1, int = 2000,
                   float = 0, float = 0, float = 0);

        std::string GetStudentId() const;
        std::string GetCourseSectionId() const;
        std::string GetID() const;

        const Date& GetEnrollmentDate() const;

        Grade& GetScore();
        const Grade& GetScore() const;

        void DisplayInfo() const;
};
