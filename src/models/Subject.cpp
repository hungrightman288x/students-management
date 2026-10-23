
#include "models/Subject.h"
#include <stdexcept>
#include <iostream>
using namespace std;

Subject::Subject(string id, string name, int credits)
    : SubjectID(id), SubjectName(name), Credits(credits) {

    if (credits < 0) {
        throw invalid_argument("So tin chi khong hop le");
    }
}

string Subject::GetID() const {
    return this->SubjectID;
}

string Subject::GetSubjectName() const {
    return this->SubjectName;
}

int Subject::GetCredits() const {
    return this->Credits;
}

void Subject::SetSubjectName(const string& name) {
    this->SubjectName = name;
}

void Subject::DisplayInfo() const {
    cout << "Subject ID: " << this->SubjectID << endl;
    cout << "Subject Name: " << this->SubjectName << endl;
    cout << "Credits: " << this->Credits << endl;
}