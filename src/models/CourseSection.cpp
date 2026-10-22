
#include "models/CourseSection.h"
#include <stdexcept>
using namespace std;

CourseSection::CourseSection(string id, string subjectId,
                             string teacherId, int maxStudents,
                             int studentNumber)
    : CourseSectionID(id),
      SubjectId(subjectId),
      TeacherId(teacherId),
      MaxStudents(maxStudents),
      StudentNumber(studentNumber) {

    if (maxStudents < 0 || studentNumber < 0 ||
        studentNumber > maxStudents) {
        throw invalid_argument("So luong sinh vien khong hop le");
    }
}

string CourseSection::GetCourseSectionId() const {
    return this->CourseSectionID;
}

string CourseSection::GetSubjectId() const {
    return this->SubjectId;
}

string CourseSection::GetTeacherId() const {
    return this->TeacherId;
}

int CourseSection::GetMaxStudents() const {
    return this->MaxStudents;
}

int CourseSection::GetStudentNumber() const {
    return this->StudentNumber;
}

void CourseSection::SetTeacher(const string& teacherID) {
    this->TeacherId = teacherID;
}

void CourseSection::IncreaseStudentNumber() {
    if (this->StudentNumber >= this->MaxStudents) {
        throw logic_error("Lop hoc phan da day");
    }

    this->StudentNumber++;
}

void CourseSection::DecreaseStudentNumber() {
    if (this->StudentNumber <= 0) {
        throw logic_error("Lop hoc phan khong co sinh vien");
    }

    this->StudentNumber--;
}
