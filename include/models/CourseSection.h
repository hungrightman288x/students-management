
#pragma once
#include <string>
using namespace std;

class CourseSection {
    private:
        string CourseSectionID;
        string SubjectId;
        string TeacherId;
        int MaxStudents;
        int StudentNumber;

    public:
        CourseSection(string = "", string = "", string = "",
                      int = 0, int = 0);

        string GetCourseSectionId() const;
        string GetSubjectId() const;
        string GetTeacherId() const;

        int GetMaxStudents() const;
        int GetStudentNumber() const;

        void SetTeacher(const string& teacherID);

        void IncreaseStudentNumber();
        void DecreaseStudentNumber();
};
