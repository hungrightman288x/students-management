#include "storage/RecordIO.h"
#include <cctype>
#include <iomanip>
#include <istream>
#include <ostream>
#include <stdexcept>
#include <system_error>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

void ValidateText(const std::string& value, const std::string& field, bool required) {
    bool hasText = false;
    for (unsigned char character : value) {
        if (std::iscntrl(character)) throw std::invalid_argument(field + " chua ky tu dieu khien");
        if (!std::isspace(character)) hasText = true;
    }
    if (required && !hasText) throw std::invalid_argument(field + " khong duoc de trong");
}

void AtomicReplaceFile(const std::filesystem::path& temporary, const std::filesystem::path& target) {
#ifdef _WIN32
    if (!MoveFileExW(temporary.c_str(), target.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
        throw std::system_error(static_cast<int>(GetLastError()), std::system_category(), "Khong thay duoc file " + target.string());
#else
    std::filesystem::rename(temporary, target);
#endif
}

void ReadRecord(std::istream& in, Account& item) {
    std::string id, password, profile;
    int role;
    if (in >> std::quoted(id) >> std::quoted(password) >> role >> std::quoted(profile)) {
        if (role < STUDENT || role > ADMIN) throw std::invalid_argument("Vai tro khong hop le");
        ValidateText(password, "Mat khau");
        item = Account(id, password, static_cast<Role>(role), profile);
    }
}

template<class T> void ReadPerson(std::istream& in, T& item) {
    std::string id, name, email;
    int day, month, year;
    if (in >> std::quoted(id) >> std::quoted(name) >> day >> month >> year >> std::quoted(email)) {
        ValidateText(name, "Ho ten");
        ValidateText(email, "Email", false);
        item = T(id, name, day, month, year, email);
    }
}
void ReadRecord(std::istream& in, Student& item) { ReadPerson(in, item); }
void ReadRecord(std::istream& in, Teacher& item) { ReadPerson(in, item); }

void ReadRecord(std::istream& in, Subject& item) {
    std::string id, name;
    int credits;
    if (in >> std::quoted(id) >> std::quoted(name) >> credits) {
        ValidateText(name, "Ten mon");
        if (credits <= 0) throw std::invalid_argument("So tin chi phai duong");
        item = Subject(id, name, credits);
    }
}

void ReadRecord(std::istream& in, CourseSection& item) {
    std::string id, subject, teacher;
    int maximum, count;
    if (in >> std::quoted(id) >> std::quoted(subject) >> std::quoted(teacher) >> maximum >> count)
        item = CourseSection(id, subject, teacher, maximum, count);
}

void ReadRecord(std::istream& in, Enrollment& item) {
    std::string id, student, section;
    int day, month, year;
    float attendance, midterm, final;
    if (in >> std::quoted(id) >> std::quoted(student) >> std::quoted(section)
           >> day >> month >> year >> attendance >> midterm >> final)
        item = Enrollment(id, student, section, day, month, year, attendance, midterm, final);
}

void WriteRecord(std::ostream& out, const Account& item) {
    out << std::quoted(item.UserName) << ' ' << std::quoted(item.Password) << ' '
        << static_cast<int>(item.UserRole) << ' ' << std::quoted(item.ProfileID);
}

template<class T> void WritePerson(std::ostream& out, const T& item) {
    const auto& birth = item.GetDateOfBirth();
    out << std::quoted(item.GetID()) << ' ' << std::quoted(item.GetFullName()) << ' '
        << birth.getDay() << ' ' << birth.getMonth() << ' ' << birth.getYear() << ' '
        << std::quoted(item.GetEmail());
}
void WriteRecord(std::ostream& out, const Student& item) { WritePerson(out, item); }
void WriteRecord(std::ostream& out, const Teacher& item) { WritePerson(out, item); }
void WriteRecord(std::ostream& out, const Subject& item) {
    out << std::quoted(item.GetID()) << ' ' << std::quoted(item.GetSubjectName()) << ' ' << item.GetCredits();
}
void WriteRecord(std::ostream& out, const CourseSection& item) {
    out << std::quoted(item.GetID()) << ' ' << std::quoted(item.GetSubjectId()) << ' '
        << std::quoted(item.GetTeacherId()) << ' ' << item.GetMaxStudents() << ' ' << item.GetStudentNumber();
}
void WriteRecord(std::ostream& out, const Enrollment& item) {
    const auto& date = item.GetEnrollmentDate();
    const auto& grade = item.GetScore();
    out << std::quoted(item.GetID()) << ' ' << std::quoted(item.GetStudentId()) << ' '
        << std::quoted(item.GetCourseSectionId()) << ' ' << date.getDay() << ' '
        << date.getMonth() << ' ' << date.getYear() << ' ' << grade.GetAttendance() << ' '
        << grade.GetMidterm() << ' ' << grade.GetFinal();
}
