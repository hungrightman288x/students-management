
#include "ui/App.h"
#include "managers/AccountManager.h"
#include "managers/StudentManager.h"
#include "managers/TeacherManager.h"
#include "managers/SubjectManager.h"
#include "managers/CourseSectionManager.h"
#include "managers/EnrollmentManager.h"

#include <commctrl.h>
#include <memory>
#include <string>
#include <vector>
#include <stdexcept>
#include <cmath>
#include <cwchar>

using namespace std;

namespace {

enum {
    LOGIN = 100,
    LOGOUT = 101,

    NAV0 = 200,
    NAV1, NAV2, NAV3, NAV4, NAV5,

    APPLY = 300,
    REFRESH = 301,
    BACK = 302,

    LIST = 400,
    FIELD = 500
};

enum class Page {
    Profile,
    Classes,
    Grades,
    Students,
    Teachers,
    Subjects,
    Sections,
    Enrollments
};

HWND wnd = nullptr;
HWND headline = nullptr;
HWND note = nullptr;
HWND list = nullptr;

HWND input[8]{};
HWND caption[8]{};
HWND nav[6]{};

HWND btnApply = nullptr;
HWND btnRefresh = nullptr;
HWND btnBack = nullptr;
HWND btnLogin = nullptr;
HWND btnLogout = nullptr;

HFONT regular = nullptr;
HFONT strong = nullptr;
HBRUSH background = nullptr;

unique_ptr<AccountManager> accounts;
unique_ptr<StudentManager> students;
unique_ptr<TeacherManager> teachers;
unique_ptr<SubjectManager> subjects;
unique_ptr<CourseSectionManager> sections;
unique_ptr<EnrollmentManager> enrollments;

Account* current = nullptr;
Page page = Page::Profile;

string activeClass;
bool editingPerson = false;
string editingId;

vector<string> selectedIds;

// =====================================================
// STRING CONVERSION
// =====================================================

wstring W(const string& s) {
    if (s.empty()) return L"";

    int n = MultiByteToWideChar(
        CP_UTF8, 0, s.c_str(), -1, nullptr, 0
    );

    if (!n)
        return wstring(s.begin(), s.end());

    wstring v(n, L'\0');

    MultiByteToWideChar(
        CP_UTF8, 0, s.c_str(), -1, &v[0], n
    );

    v.pop_back();
    return v;
}

string U(const wstring& s) {
    if (s.empty()) return "";

    int n = WideCharToMultiByte(
        CP_UTF8, 0, s.c_str(), -1,
        nullptr, 0, nullptr, nullptr
    );

    string v(n, '\0');

    WideCharToMultiByte(
        CP_UTF8, 0, s.c_str(), -1,
        &v[0], n, nullptr, nullptr
    );

    v.pop_back();
    return v;
}

wstring txt(HWND h) {
    int n = GetWindowTextLengthW(h);
    wstring t(n + 1, L'\0');

    GetWindowTextW(h, &t[0], n + 1);
    t.resize(n);

    return t;
}

void set(HWND h, const wstring& t) {
    SetWindowTextW(h, t.c_str());
}

void error(const string& e) {
    MessageBoxW(
        wnd,
        W(e).c_str(),
        L"Student Management - Error",
        MB_OK | MB_ICONERROR
    );
}

void info(const wstring& s) {
    set(note, s);
}

// =====================================================
// CREATE CONTROLS
// =====================================================

HWND make(
    const wchar_t* cls,
    const wchar_t* text,
    int x, int y,
    int w, int h,
    int id = 0,
    DWORD style = 0,
    DWORD extra = 0
) {
    HWND z = CreateWindowExW(
        extra,
        cls,
        text,
        WS_CHILD | WS_VISIBLE | style,
        x, y, w, h,
        wnd,
        (HMENU)(INT_PTR)id,
        GetModuleHandleW(nullptr),
        nullptr
    );

    SendMessageW(
        z, WM_SETFONT,
        (WPARAM)regular, TRUE
    );

    return z;
}

void show(HWND h, bool yes) {
    ShowWindow(h, yes ? SW_SHOW : SW_HIDE);
}

// =====================================================
// LIST VIEW
// =====================================================

void col(int i, const wchar_t* name, int width) {
    LVCOLUMNW c{};

    c.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    c.pszText = (LPWSTR)name;
    c.cx = width;
    c.iSubItem = i;

    ListView_InsertColumn(list, i, &c);
}

void row(
    const vector<wstring>& cells,
    const string& id = ""
) {
    if (cells.empty()) return;

    int i = ListView_GetItemCount(list);

    LVITEMW it{};
    it.mask = LVIF_TEXT;
    it.iItem = i;
    it.pszText = (LPWSTR)cells[0].c_str();

    ListView_InsertItem(list, &it);

    for (int c = 1; c < (int)cells.size(); ++c) {
        ListView_SetItemText(
            list,
            i,
            c,
            (LPWSTR)cells[c].c_str()
        );
    }

    selectedIds.push_back(id);
}

void clearList() {
    ListView_DeleteAllItems(list);

    while (ListView_DeleteColumn(list, 0)) {}

    selectedIds.clear();
}

string picked() {
    int i = ListView_GetNextItem(
        list, -1, LVNI_SELECTED
    );

    if (i >= 0 && i < (int)selectedIds.size())
        return selectedIds[i];

    return "";
}

// =====================================================
// INPUT VALIDATION
// =====================================================

string val(int i) {
    return U(txt(input[i]));
}

int integer(int i) {
    string s = val(i);
    size_t n = 0;

    int x = stoi(s, &n);

    if (n != s.size())
        throw invalid_argument("Invalid integer");

    return x;
}

float decimal(int i) {
    string s = val(i);
    size_t n = 0;

    float x = stof(s, &n);

    if (
        n != s.size() ||
        !std::isfinite(x) ||
        x < 0 ||
        x > 10
    ) {
        throw invalid_argument(
            "Grade must be between 0 and 10"
        );
    }

    return x;
}

void noPipe(const string& s) {
    if (
        s.empty() ||
        s.find_first_of("|\r\n") != string::npos
    ) {
        throw invalid_argument(
            "Empty field or invalid separator"
        );
    }
}

wstring num(float f) {
    wchar_t b[40];

    swprintf(b, 40, L"%.2f", (double)f);

    return b;
}

// =====================================================
// CONFIGURE INPUT FIELDS
// =====================================================

void configure(
    const vector<wstring>& labels,
    const wstring& action
) {
    for (int i = 0; i < 8; ++i) {
        bool yes = i < (int)labels.size();

        show(caption[i], yes);
        show(input[i], yes);

        if (yes) {
            set(caption[i], labels[i]);
            set(input[i], L"");
        }
    }

    show(btnApply, !action.empty());
    set(btnApply, action);
}

// =====================================================
// LOAD MANAGERS
// =====================================================

void load() {
    students = make_unique<StudentManager>(
        "data/students.txt"
    );

    teachers = make_unique<TeacherManager>(
        "data/teachers.txt"
    );

    subjects = make_unique<SubjectManager>(
        "data/subjects.txt"
    );

    sections = make_unique<CourseSectionManager>(
        "data/course_sections.txt",
        *subjects,
        *teachers
    );

    enrollments = make_unique<EnrollmentManager>(
        "data/enrollments.txt",
        *students,
        *sections
    );

    accounts = make_unique<AccountManager>(
        "data/accounts.txt"
    );
}

// =====================================================
// NAVIGATION
// =====================================================



void navSetup() {
    vector<wstring> labels;

    if (current->GetUserRole() == STUDENT) {
        labels = {
            L"My profile",
            L"My classes",
            L"My grades"
        };
    }
    else if (current->GetUserRole() == TEACHER) {
        labels = {
            L"My profile",
            L"My classes"
        };
    }
    else {
        labels = {
            L"Students",
            L"Teachers",
            L"Subjects",
            L"Sections",
            L"Enrollments",
            L"Grades (view)"
        };
    }

    for (int i = 0; i < 6; ++i) {
        show(nav[i], i < (int)labels.size());

        if (i < (int)labels.size()) {
            set(nav[i], labels[i]);
        }
    }
}



// =====================================================
// DISPLAY PAGE
// =====================================================

void enter(Page next) {

    SendMessageW(
        input[1],
        EM_SETPASSWORDCHAR,
        0, 0
    );

    for (int i = 0; i < 8; ++i) {
        int x = 225 + (i % 4) * 185;
        int y = 420 + (i / 4) * 68;

        MoveWindow(
            caption[i], x, y,
            175, 20, TRUE
        );

        MoveWindow(
            input[i], x, y + 20,
            169, 28, TRUE
        );
    }

    page = next;

    editingPerson = false;
    editingId.clear();

    SendMessageW(
        input[0],
        EM_SETREADONLY,
        FALSE, 0
    );

    show(btnBack, false);

    clearList();

    show(list, true);
    show(btnRefresh, true);
    show(btnLogout, true);

    for (int i = 0; i < 8; ++i) {
        show(input[i], false);
        show(caption[i], false);
    }

    show(btnApply, false);

    set(headline, L"Student Management");
    info(L"Select an item to continue.");

    Role r = current->GetUserRole();
    string id = current->GetID();

    // =================================================
    // PROFILE
    // =================================================

    if (next == Page::Profile) {
        set(headline, L"Personal profile");

        col(0, L"ID", 120);
        col(1, L"Name", 250);
        col(2, L"Date of birth", 130);
        col(3, L"Email", 240);

        if (r == STUDENT) {
            Student* s = students->FindByID(id);

            if (!s)
                throw runtime_error("Student profile missing");

            row({
                W(s->GetID()),
                W(s->GetFullName()),
                W(s->GetDateOfBirth().to_string()),
                W(s->GetEmail())
            }, id);

            info(L"Your personal information is read-only.");
        }
        else if (r == TEACHER) {
            Teacher* t = teachers->FindByID(id);

            if (!t)
                throw runtime_error("Teacher profile missing");

            row({
                W(t->GetID()),
                W(t->GetFullName()),
                W(t->GetDateOfBirth().to_string()),
                W(t->GetEmail())
            }, id);

            configure(
                {L"New full name", L"New email"},
                L"Update my profile"
            );

            info(
                L"Leave a field empty to keep its current value."
            );
        }
    }

    // =================================================
    // CLASSES
    // =================================================

    else if (next == Page::Classes) {
        set(headline, L"Course sections");

        col(0, L"Section", 110);
        col(1, L"Subject", 260);
        col(2, L"Teacher", 255);
        col(3, L"Students", 95);

        for (const auto& kv : sections->GetAll()) {
            const CourseSection* c = kv.second;

            if (
                r == TEACHER &&
                c->GetTeacherId() != id
            ) continue;

            if (r == STUDENT) {
                bool belongs = false;

                for (const auto& ekv : enrollments->GetAll()) {
                    const Enrollment* e = ekv.second;

                    if (
                        e->GetStudentId() == id &&
                        e->GetCourseSectionId() == c->GetID()
                    ) {
                        belongs = true;
                        break;
                    }
                }

                if (!belongs) continue;
            }

            const Subject* s = subjects->FindByID(
                c->GetSubjectId()
            );

            const Teacher* t = teachers->FindByID(
                c->GetTeacherId()
            );

            row({
                W(c->GetID()),
                s ? W(s->GetSubjectName()) : W(c->GetSubjectId()),
                t ? W(t->GetFullName()) : W(c->GetTeacherId()),
                to_wstring(c->GetStudentNumber())
            }, c->GetID());
        }

        if (r == TEACHER) {
            configure(
                {},
                L"View students in selected class"
            );

            info(
                L"Select a class and click View students, "
                L"or double-click a class."
            );
        }
        else {
            info(L"Your registered course sections.");
        }
    }

    // =================================================
    // GRADES
    // =================================================

    else if (next == Page::Grades) {

        if (r == TEACHER) {
            if (activeClass.empty()) {
                enter(Page::Classes);

                info(L"Please select a class first.");
                return;
            }

            const CourseSection* c =
                sections->FindByID(activeClass);

            if (!c || c->GetTeacherId() != id) {
                throw runtime_error(
                    "You do not teach this class"
                );
            }

            set(
                headline,
                L"Students and grades - " + W(activeClass)
            );
        }
        else {
            set(headline, L"Grades");
        }

        // Only student name, subject and grades.
        col(0, L"No.", 55);
        col(1, L"Student", 195);
        col(2, L"Attendance", 110);
        col(3, L"Midterm", 100);
        col(4, L"Final", 100);
        col(5, L"Total", 100);
        
        int index = 0;

        for (const auto& kv : enrollments->GetAll()) {
            const Enrollment* e = kv.second;

            const CourseSection* c =
                sections->FindByID(
                    e->GetCourseSectionId()
                );

            if (!c) continue;

            if (
                r == STUDENT &&
                e->GetStudentId() != id
            ) continue;

            if (
                r == TEACHER &&
                (
                    c->GetTeacherId() != id ||
                    c->GetID() != activeClass
                )
            ) continue;

            const Student* s =
                students->FindByID(e->GetStudentId());

            const Subject* subject =
                subjects->FindByID(c->GetSubjectId());

            const Grade& g = e->GetScore();

            row({
                to_wstring(++index),
                s ? W(s->GetFullName()) : W(e->GetStudentId()),
                num(g.GetAttendance()),
                num(g.GetMidterm()),
                num(g.GetFinal()),
                num(g.GetTotal())
            }, e->GetID());
        }

        if (r == TEACHER) {
            configure(
                {
                    L"Attendance (0-10)",
                    L"Midterm (0-10)",
                    L"Final (0-10)"
                },
                L"Save grades"
            );

            show(btnBack, true);

            info(
                L"Select a student, edit the scores, "
                L"then click Save."
            );
        }
        else {
            info(L"Grades are read-only.");
        }
    }

    // =================================================
    // ADMIN - STUDENTS
    // =================================================

    else if (next == Page::Students) {
        set(headline, L"Manage students");

        col(0, L"Student ID", 115);
        col(1, L"Full name", 205);
        col(2, L"Date of birth", 130);
        col(3, L"Email", 290);

        for (const auto& kv : students->GetAll()) {
            const Student* s = kv.second;

            row({
                W(s->GetID()),
                W(s->GetFullName()),
                W(s->GetDateOfBirth().to_string()),
                W(s->GetEmail())
            }, s->GetID());
        }

        configure(
            {
                L"Student ID (SVxxx)",
                L"Full name",
                L"Day",
                L"Month",
                L"Year",
                L"Email"
            },
            L"Add student"
        );

        info(
            L"Fill all fields to add a student. "
            L"Select a row to update an existing student."
        );
    }

    // =================================================
    // ADMIN - TEACHERS
    // =================================================

    else if (next == Page::Teachers) {
        set(headline, L"Manage teachers");

        col(0, L"Teacher ID", 115);
        col(1, L"Full name", 205);
        col(2, L"Date of birth", 130);
        col(3, L"Email", 290);

        for (const auto& kv : teachers->GetAll()) {
            const Teacher* t = kv.second;

            row({
                W(t->GetID()),
                W(t->GetFullName()),
                W(t->GetDateOfBirth().to_string()),
                W(t->GetEmail())
            }, t->GetID());
        }

        configure(
            {
                L"Teacher ID (GVxxx)",
                L"Full name",
                L"Day",
                L"Month",
                L"Year",
                L"Email"
            },
            L"Add teacher"
        );

        info(
            L"Fill all fields to add a teacher. "
            L"Select a row to update an existing teacher."
        );
    }

    // =================================================
    // ADMIN - SUBJECTS
    // =================================================

    else if (next == Page::Subjects) {
        set(headline, L"Manage subjects");

        col(0, L"Subject ID", 150);
        col(1, L"Subject name", 370);
        col(2, L"Credits", 100);

        for (const auto& kv : subjects->GetAll()) {
            const Subject* s = kv.second;

            row({
                W(s->GetID()),
                W(s->GetSubjectName()),
                to_wstring(s->GetCredits())
            }, s->GetID());
        }

        configure(
            {
                L"Subject ID (MHxxx)",
                L"Subject name",
                L"Credits"
            },
            L"Add subject"
        );
    }

    // =================================================
    // ADMIN - SECTIONS
    // =================================================

    else if (next == Page::Sections) {
        set(headline, L"Manage course sections");

        col(0, L"Section", 110);
        col(1, L"Subject", 120);
        col(2, L"Teacher", 250);
        col(3, L"Capacity", 100);
        col(4, L"Current", 90);

        for (const auto& kv : sections->GetAll()) {
            const CourseSection* c = kv.second;

            const Teacher* t =
                teachers->FindByID(c->GetTeacherId());

            row({
                W(c->GetID()),
                W(c->GetSubjectId()),
                t ? W(t->GetFullName()) : W(c->GetTeacherId()),
                to_wstring(c->GetMaxStudents()),
                to_wstring(c->GetStudentNumber())
            }, c->GetID());
        }

        configure(
            {
                L"Section ID (LHPxxx)",
                L"Subject ID",
                L"Teacher ID",
                L"Capacity"
            },
            L"Add empty section"
        );
    }

    // =================================================
    // ADMIN - ENROLLMENTS
    // =================================================

    else if (next == Page::Enrollments) {
        set(headline, L"Manage enrollments");

        col(0, L"Enrollment", 120);
        col(1, L"Student", 110);
        col(2, L"Section", 110);
        col(3, L"Date", 150);

        for (const auto& kv : enrollments->GetAll()) {
            const Enrollment* e = kv.second;

            row({
                W(e->GetID()),
                W(e->GetStudentId()),
                W(e->GetCourseSectionId()),
                W(e->GetEnrollmentDate().to_string())
            }, e->GetID());
        }

        configure(
            {
                L"Enrollment ID (DKxxx)",
                L"Student ID",
                L"Section ID",
                L"Day",
                L"Month",
                L"Year"
            },
            L"Register student"
        );

        info(
            L"Admin can register students "
            L"but cannot edit their grades."
        );
    }
}

// =====================================================
// CHOOSE NAVIGATION
// =====================================================

void chooseNav(int i) {
    Role r = current->GetUserRole();

    if (r == ADMIN) {
        static Page pages[] = {
            Page::Students,
            Page::Teachers,
            Page::Subjects,
            Page::Sections,
            Page::Enrollments,
            Page::Grades
        };

        enter(pages[i]);
    }
    else {
        static Page pages[] = {
            Page::Profile,
            Page::Classes,
            Page::Grades
        };

        enter(pages[i]);
    }
}

// =====================================================
// TEACHER - OPEN CLASS
// =====================================================

void openSelectedClass() {
    if (
        !current ||
        current->GetUserRole() != TEACHER ||
        page != Page::Classes
    ) return;

    string cid = picked();

    if (cid.empty()) {
        throw runtime_error(
            "Please select a class first"
        );
    }

    const CourseSection* c = sections->FindByID(cid);

    if (
        !c ||
        c->GetTeacherId() != current->GetID()
    ) {
        throw runtime_error(
            "You do not teach this class"
        );
    }

    activeClass = cid;

    enter(Page::Grades);
}

// =====================================================
// TEACHER - SELECT STUDENT
// =====================================================

void loadSelectedStudentGrades() {
    if (
        !current ||
        current->GetUserRole() != TEACHER ||
        page != Page::Grades
    ) return;

    string eid = picked();

    if (eid.empty()) return;

    Enrollment* e = enrollments->FindByID(eid);

    if (
        !e ||
        e->GetCourseSectionId() != activeClass
    ) {
        throw runtime_error(
            "Invalid enrollment selection"
        );
    }

    const Student* student =
        students->FindByID(e->GetStudentId());

    if (!student) {
        throw runtime_error("Student not found");
    }

    const Grade& g = e->GetScore();

    set(input[0], num(g.GetAttendance()));
    set(input[1], num(g.GetMidterm()));
    set(input[2], num(g.GetFinal()));

    info(
        L"Selected student: " +
        W(student->GetFullName()) +
        L". Edit the scores and click Save."
    );
}

// =====================================================
// ADMIN - SELECT STUDENT OR TEACHER
// =====================================================

void selectPerson() {
    if (
        !current ||
        current->GetUserRole() != ADMIN
    ) return;

    if (
        page != Page::Students &&
        page != Page::Teachers
    ) return;

    string id = picked();

    if (id.empty()) return;

    string name;
    string email;
    wstring date;

    if (page == Page::Students) {
        const Student* s = students->FindByID(id);

        if (!s)
            throw runtime_error("Student not found");

        name = s->GetFullName();
        email = s->GetEmail();
        date = W(s->GetDateOfBirth().to_string());
    }
    else {
        const Teacher* t = teachers->FindByID(id);

        if (!t)
            throw runtime_error("Teacher not found");

        name = t->GetFullName();
        email = t->GetEmail();
        date = W(t->GetDateOfBirth().to_string());
    }

    editingPerson = true;
    editingId = id;

    set(input[0], W(id));

    for (int i = 1; i < 6; ++i) {
        set(input[i], L"");
    }

    // An 3 o ngay, thang, nam khi cap nhat
    for (int i = 2; i <= 4; ++i) {
        show(input[i], false);
        show(caption[i], false);
    }

    // Primary key cannot be modified.
    SendMessageW(
        input[0],
        EM_SETREADONLY,
        TRUE, 0
    );

    set(btnApply, L"Update information");
    show(btnBack, true);

    info(
        L"Selected: " + W(name) +
        L" | DOB: " + date +
        L" | Email: " + W(email) +
        L". Empty fields keep old values."
    );
}

// =====================================================
// SAVE / APPLY
// =====================================================

void apply() {
    if (
        page == Page::Classes &&
        current &&
        current->GetUserRole() == TEACHER
    ) {
        openSelectedClass();
        return;
    }

    if (!current)
        throw runtime_error("Login required");

    Role r = current->GetUserRole();

    // =================================================
    // TEACHER UPDATE PROFILE
    // =================================================

    if (page == Page::Profile && r == TEACHER) {
        Teacher* t = teachers->FindByID(current->GetID());

        if (!t)
            throw runtime_error("Teacher missing");

        string name = val(0);
        string email = val(1);

        if (!name.empty()) noPipe(name);
        if (!email.empty()) noPipe(email);

        Teacher old = *t;

        try {
            if (!name.empty())
                t->SetFullName(name);

            if (!email.empty())
                t->SetEmail(email);

            teachers->SaveToFile();
        }
        catch (...) {
            *t = old;
            throw;
        }
    }

    // =================================================
    // TEACHER UPDATE GRADES
    // =================================================

    else if (page == Page::Grades && r == TEACHER) {
        string eid = picked();

        if (eid.empty()) {
            throw runtime_error(
                "Select a student first"
            );
        }

        Enrollment* e = enrollments->FindByID(eid);

        if (!e)
            throw runtime_error("Enrollment missing");

        const CourseSection* c =
            sections->FindByID(e->GetCourseSectionId());

        if (
            !c ||
            c->GetTeacherId() != current->GetID() ||
            c->GetID() != activeClass
        ) {
            throw runtime_error(
                "You do not teach this class"
            );
        }

        float a = decimal(0);
        float m = decimal(1);
        float f = decimal(2);

        Grade previous = e->GetScore();

        e->GetScore().SetAttendance(a);
        e->GetScore().SetMidterm(m);
        e->GetScore().SetFinal(f);

        try {
            enrollments->SaveToFile();
        }
        catch (...) {
            e->GetScore() = previous;
            throw;
        }
    }

    // =================================================
    // ADMIN STUDENTS
    // =================================================

    else if (r == ADMIN && page == Page::Students) {

        if (editingPerson) {
            Student* s = students->FindByID(editingId);

            if (!s)
                throw runtime_error("Student not found");

            string name = val(1);
            string email = val(5);

            if (!name.empty()) noPipe(name);
            if (!email.empty()) noPipe(email);


            Student previous = *s;

            try {
                if (!name.empty())
                    s->SetFullName(name);

                if (!email.empty())
                    s->SetEmail(email);

                students->SaveToFile();
            }
            catch (...) {
                *s = previous;
                throw;
            }
        }
        else {
            string id = val(0);
            string name = val(1);
            string email = val(5);

            noPipe(id);
            noPipe(name);
            noPipe(email);

            unique_ptr<Student> p(
                new Student(
                    id, name,
                    integer(2),
                    integer(3),
                    integer(4),
                    email
                )
            );

            if (!students->Add(p.get())) {
                throw runtime_error(
                    "Duplicate student ID"
                );
            }

            p.release();
            students->SaveToFile();
        }
    }

    // =================================================
    // ADMIN TEACHERS
    // =================================================

    else if (r == ADMIN && page == Page::Teachers) {

        if (editingPerson) {
            Teacher* t = teachers->FindByID(editingId);

            if (!t)
                throw runtime_error("Teacher not found");

            string name = val(1);
            string email = val(5);

            if (!name.empty()) noPipe(name);
            if (!email.empty()) noPipe(email);

            Teacher previous = *t;

            try {
                if (!name.empty())
                    t->SetFullName(name);

                if (!email.empty())
                    t->SetEmail(email);

                teachers->SaveToFile();
            }
            catch (...) {
                *t = previous;
                throw;
            }
        }
        else {
            string id = val(0);
            string name = val(1);
            string email = val(5);

            noPipe(id);
            noPipe(name);
            noPipe(email);

            unique_ptr<Teacher> p(
                new Teacher(
                    id, name,
                    integer(2),
                    integer(3),
                    integer(4),
                    email
                )
            );

            if (!teachers->Add(p.get())) {
                throw runtime_error(
                    "Duplicate teacher ID"
                );
            }

            p.release();
            teachers->SaveToFile();
        }
    }

    // =================================================
    // ADMIN SUBJECTS
    // =================================================

    else if (r == ADMIN && page == Page::Subjects) {
        string id = val(0);
        string name = val(1);

        noPipe(id);
        noPipe(name);

        unique_ptr<Subject> p(
            new Subject(id, name, integer(2))
        );

        if (!subjects->Add(p.get())) {
            throw runtime_error(
                "Duplicate subject ID"
            );
        }

        p.release();
        subjects->SaveToFile();
    }

    // =================================================
    // ADMIN SECTIONS
    // =================================================

    else if (r == ADMIN && page == Page::Sections) {
        string id = val(0);
        string sid = val(1);
        string tid = val(2);

        noPipe(id);
        noPipe(sid);
        noPipe(tid);

        if (
            !subjects->FindByID(sid) ||
            !teachers->FindByID(tid)
        ) {
            throw runtime_error(
                "Subject or teacher missing"
            );
        }

        unique_ptr<CourseSection> p(
            new CourseSection(
                id, sid, tid,
                integer(3), 0
            )
        );

        if (!sections->Add(p.get())) {
            throw runtime_error(
                "Duplicate section ID"
            );
        }

        p.release();
        sections->SaveToFile();
    }

    // =================================================
    // ADMIN ENROLLMENTS
    // =================================================

    else if (r == ADMIN && page == Page::Enrollments) {
        string id = val(0);
        string sid = val(1);
        string cid = val(2);

        noPipe(id);
        noPipe(sid);
        noPipe(cid);

        if (!students->FindByID(sid)) {
            throw runtime_error("Student missing");
        }

        CourseSection* c =
            sections->FindByID(cid);

        if (!c) {
            throw runtime_error("Section missing");
        }

        for (const auto& kv : enrollments->GetAll()) {
            const Enrollment* e = kv.second;

            if (
                e->GetStudentId() == sid &&
                e->GetCourseSectionId() == cid
            ) {
                throw runtime_error(
                    "Student already registered"
                );
            }
        }

        if (
            c->GetStudentNumber() >=
            c->GetMaxStudents()
        ) {
            throw runtime_error("Section is full");
        }

        unique_ptr<Enrollment> p(
            new Enrollment(
                id, sid, cid,
                integer(3),
                integer(4),
                integer(5),
                0, 0, 0
            )
        );

        if (!enrollments->Add(p.get())) {
            throw runtime_error(
                "Duplicate enrollment ID"
            );
        }

        p.release();

        c->IncreaseStudentNumber();

        enrollments->SaveToFile();
        sections->SaveToFile();
    }
    else {
        throw runtime_error(
            "Operation not allowed"
        );
    }

    enter(page);

    info(L"Saved successfully.");
}

// =====================================================
// LOGIN
// =====================================================

void login() {
    if (!accounts) {
        throw runtime_error("Data was not loaded");
    }

    string user = val(0);
    string pass = val(1);

    Account* a = accounts->Login(user, pass);

    if (!a) {
        throw runtime_error(
            "Wrong username or password"
        );
    }

    if (
        a->GetUserRole() == STUDENT &&
        !students->FindByID(a->GetID())
    ) {
        throw runtime_error(
            "Student account has no profile"
        );
    }

    if (
        a->GetUserRole() == TEACHER &&
        !teachers->FindByID(a->GetID())
    ) {
        throw runtime_error(
            "Teacher account has no profile"
        );
    }

    current = a;

    show(btnLogin, false);
    navSetup();
    show(btnLogout, true);

    chooseNav(0);
}

// =====================================================
// LOGOUT
// =====================================================

void logout() {
    activeClass.clear();

    current = nullptr;
    editingPerson = false;
    editingId.clear();

    clearList();

    show(list, false);
    show(btnRefresh, false);
    show(btnBack, false);
    show(btnLogout, false);
    show(btnApply, false);

    for (auto n : nav)
        show(n, false);

    set(headline, L"STUDENT MANAGEMENT");

    info(
        L"Log in using your username and password."
    );

    configure(
        {L"Username", L"Password"},
        L""
    );

    for (int i = 0; i < 2; ++i) {
        MoveWindow(
            caption[i],
            225,
            130 + i * 68,
            350, 22, TRUE
        );

        MoveWindow(
            input[i],
            225,
            156 + i * 68,
            330, 32, TRUE
        );
    }

    SendMessageW(
        input[1],
        EM_SETPASSWORDCHAR,
        (WPARAM)L'\x25cf',
        0
    );

    InvalidateRect(input[1], nullptr, TRUE);

    show(btnLogin, true);
}

// =====================================================
// WINDOW PROCEDURE
// =====================================================

LRESULT CALLBACK windowProc(
    HWND h,
    UINT msg,
    WPARAM wp,
    LPARAM lp
) {
    switch (msg) {

    case WM_CREATE: {
        wnd = h;

        regular = CreateFontW(
            -17, 0, 0, 0,
            FW_NORMAL,
            0, 0, 0,
            DEFAULT_CHARSET,
            0, 0,
            CLEARTYPE_QUALITY,
            0,
            L"Segoe UI"
        );

        strong = CreateFontW(
            -28, 0, 0, 0,
            FW_BOLD,
            0, 0, 0,
            DEFAULT_CHARSET,
            0, 0,
            CLEARTYPE_QUALITY,
            0,
            L"Segoe UI"
        );

        background = CreateSolidBrush(
            RGB(245, 247, 252)
        );

        headline = make(
            L"STATIC",
            L"STUDENT MANAGEMENT",
            25, 22, 820, 44
        );

        SendMessageW(
            headline,
            WM_SETFONT,
            (WPARAM)strong,
            TRUE
        );

        // Status text moved below the Save button.
        note = make(
            L"STATIC", L"",
            225, 650, 735, 56
        );

        for (int i = 0; i < 6; ++i) {
            nav[i] = make(
                L"BUTTON", L"",
                20,
                105 + i * 53,
                175, 43,
                NAV0 + i,
                BS_PUSHBUTTON
            );
        }

        btnLogout = make(
            L"BUTTON", L"Logout",
            840, 22, 110, 38,
            LOGOUT,
            BS_PUSHBUTTON
        );

        list = make(
            WC_LISTVIEWW, L"",
            225, 105, 740, 300,
            LIST,
            LVS_REPORT |
            LVS_SINGLESEL |
            WS_BORDER,
            WS_EX_CLIENTEDGE
        );

        ListView_SetExtendedListViewStyle(
            list,
            LVS_EX_FULLROWSELECT |
            LVS_EX_GRIDLINES |
            LVS_EX_DOUBLEBUFFER
        );

        for (int i = 0; i < 8; ++i) {
            int x = 225 + (i % 4) * 185;
            int y = 420 + (i / 4) * 68;

            caption[i] = make(
                L"STATIC", L"",
                x, y, 175, 20
            );

            input[i] = make(
                L"EDIT", L"",
                x, y + 20, 169, 28,
                FIELD + i,
                WS_BORDER |
                ES_AUTOHSCROLL |
                WS_TABSTOP,
                WS_EX_CLIENTEDGE
            );
        }

        btnApply = make(
            L"BUTTON", L"Save",
            225, 580, 270, 40,
            APPLY,
            BS_PUSHBUTTON
        );

        btnRefresh = make(
            L"BUTTON", L"Refresh",
            20, 453, 175, 40,
            REFRESH,
            BS_PUSHBUTTON
        );

        btnBack = make(
            L"BUTTON", L"Back to list",
            20, 508, 175, 40,
            BACK,
            BS_PUSHBUTTON
        );

        btnLogin = make(
            L"BUTTON", L"Login",
            225, 280, 160, 40,
            LOGIN,
            BS_DEFPUSHBUTTON
        );

        logout();

        try {
            load();
        }
        catch (const exception& e) {
            error(
                string("Cannot load data: ") + e.what()
            );
        }

        return 0;
    }

    case WM_COMMAND: {
        int id = LOWORD(wp);

        try {
            if (id == LOGIN) {
                login();
            }
            else if (id == LOGOUT) {
                logout();
            }
            else if (
                id >= NAV0 &&
                id <= NAV5 &&
                current
            ) {
                chooseNav(id - NAV0);
            }
            else if (id == APPLY) {
                apply();
            }
            else if (id == REFRESH && current) {
                enter(page);
            }
            else if (id == BACK && current) {
                if (
                    current->GetUserRole() == TEACHER &&
                    page == Page::Grades
                ) {
                    activeClass.clear();
                    enter(Page::Classes);
                }
                else if (
                    page == Page::Students ||
                    page == Page::Teachers
                ) {
                    enter(page);
                }
                else {
                    chooseNav(0);
                }
            }
        }
        catch (const exception& e) {
            error(e.what());
        }

        return 0;
    }

    case WM_NOTIFY: {
        const NMHDR* hdr = (const NMHDR*)lp;

        if (
            hdr &&
            hdr->idFrom == LIST &&
            current
        ) {
            try {
                // Open selected class.
                if (
                    hdr->code == NM_DBLCLK &&
                    page == Page::Classes &&
                    current->GetUserRole() == TEACHER
                ) {
                    openSelectedClass();
                }

                // Teacher selects student.
                else if (
                    hdr->code == LVN_ITEMCHANGED &&
                    page == Page::Grades &&
                    current->GetUserRole() == TEACHER
                ) {
                    const NMLISTVIEW* evt =
                        (const NMLISTVIEW*)lp;

                    if (
                        (evt->uChanged & LVIF_STATE) &&
                        (evt->uNewState & LVIS_SELECTED) &&
                        !(evt->uOldState & LVIS_SELECTED)
                    ) {
                        loadSelectedStudentGrades();
                    }
                }

                // Admin selects student or teacher.
                else if (
                    hdr->code == LVN_ITEMCHANGED &&
                    current->GetUserRole() == ADMIN &&
                    (
                        page == Page::Students ||
                        page == Page::Teachers
                    )
                ) {
                    const NMLISTVIEW* evt =
                        (const NMLISTVIEW*)lp;

                    if (
                        (evt->uChanged & LVIF_STATE) &&
                        (evt->uNewState & LVIS_SELECTED) &&
                        !(evt->uOldState & LVIS_SELECTED)
                    ) {
                        selectPerson();
                    }
                }
            }
            catch (const exception& e) {
                error(e.what());
            }
        }

        return 0;
    }

    case WM_ERASEBKGND: {
        RECT rc;
        GetClientRect(h, &rc);

        FillRect(
            (HDC)wp,
            &rc,
            background
        );

        return 1;
    }

    case WM_CTLCOLORSTATIC: {
        SetBkMode(
            (HDC)wp,
            TRANSPARENT
        );

        SetTextColor(
            (HDC)wp,
            RGB(29, 50, 83)
        );

        return (LRESULT)background;
    }

    case WM_DESTROY: {
        accounts.reset();
        enrollments.reset();
        sections.reset();
        subjects.reset();
        teachers.reset();
        students.reset();

        DeleteObject(regular);
        DeleteObject(strong);
        DeleteObject(background);

        PostQuitMessage(0);

        return 0;
    }
    }

    return DefWindowProcW(
        h, msg, wp, lp
    );
}

} // namespace

// =====================================================
// APPLICATION ENTRY
// =====================================================

int RunApplication(
    HINSTANCE instance,
    int showCommand
) {
    INITCOMMONCONTROLSEX cc{
        sizeof(cc),
        ICC_LISTVIEW_CLASSES
    };

    InitCommonControlsEx(&cc);

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.hInstance = instance;
    wc.lpfnWndProc = windowProc;
    wc.lpszClassName = L"SMS2026Desktop";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    if (!RegisterClassExW(&wc))
        return 1;

    HWND h = CreateWindowExW(
        0,
        wc.lpszClassName,
        L"Student Management System",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        1000, 790,
        nullptr,
        nullptr,
        instance,
        nullptr
    );

    if (!h)
        return 1;

    ShowWindow(h, showCommand);
    UpdateWindow(h);

    MSG m{};

    while (
        GetMessageW(
            &m,
            nullptr,
            0, 0
        ) > 0
    ) {
        TranslateMessage(&m);
        DispatchMessageW(&m);
    }

    return (int)m.wParam;
}
