
#include "models/Enrollment.h"
#include <iostream>
using namespace std;

Enrollment::Enrollment(string id, string studentId,
                       string courseSectionId,
                       int day, int month, int year,
                       float att, float mid, float final)
    : EnrollmentID(id),
      StudentID(studentId),
      CourseSectionID(courseSectionId),
      EnrollmentDate(day, month, year),
      Score(att, mid, final) {}

string Enrollment::GetStudentId() const {
    return this->StudentID;
}

string Enrollment::GetCourseSectionId() const {
    return this->CourseSectionID;
}

string Enrollment::GetID() const {
    return this->EnrollmentID;
}

const Date& Enrollment::GetEnrollmentDate() const {
    return this->EnrollmentDate;
}

Grade& Enrollment::GetScore() {
    return this->Score;
}

const Grade& Enrollment::GetScore() const {
    return this->Score;
}


void Enrollment::DisplayInfo() const {
    cout << "Enrollment ID: " << this->EnrollmentID << endl;
    cout << "Student ID: " << this->StudentID << endl;
    cout << "Course Section ID: " << this->CourseSectionID << endl;
    cout << "Enrollment Date: " << this->EnrollmentDate.to_string() << endl;
    cout << "Attendance: " << this->Score.GetAttendance() << endl;
    cout << "Midterm: " << this->Score.GetMidterm() << endl;
    cout << "Final: " << this->Score.GetFinal() << endl;
    cout << "Total: " << this->Score.GetTotal() << endl;
}