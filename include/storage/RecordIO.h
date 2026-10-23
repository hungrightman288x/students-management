#pragma once

#include <filesystem>
#include <iosfwd>
#include <string>
#include "models/Account.h"
#include "models/CourseSection.h"
#include "models/Enrollment.h"
#include "models/Student.h"
#include "models/Subject.h"
#include "models/Teacher.h"

void ReadRecord(std::istream&, Account&);
void ReadRecord(std::istream&, Student&);
void ReadRecord(std::istream&, Teacher&);
void ReadRecord(std::istream&, Subject&);
void ReadRecord(std::istream&, CourseSection&);
void ReadRecord(std::istream&, Enrollment&);
void WriteRecord(std::ostream&, const Account&);
void WriteRecord(std::ostream&, const Student&);
void WriteRecord(std::ostream&, const Teacher&);
void WriteRecord(std::ostream&, const Subject&);
void WriteRecord(std::ostream&, const CourseSection&);
void WriteRecord(std::ostream&, const Enrollment&);

void ValidateText(const std::string& value, const std::string& field, bool required = true);
void AtomicReplaceFile(const std::filesystem::path& temporary, const std::filesystem::path& target);
