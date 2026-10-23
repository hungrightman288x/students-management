
#pragma once
#include <string>

class CourseSection {
    private:
        std::string CourseSectionID;
        std::string SubjectId;
        std::string TeacherId;
        int MaxStudents;
        int StudentNumber;

    public:
        CourseSection(std::string = "", std::string = "", std::string = "",
                      int = 0, int = 0);

        std::string GetID() const;
        std::string GetSubjectId() const;
        std::string GetTeacherId() const;

        int GetMaxStudents() const;
        int GetStudentNumber() const;

        void SetTeacher(const std::string& teacherID);

        void IncreaseStudentNumber();
        void DecreaseStudentNumber();

        void DisplayInfo() const;
};
