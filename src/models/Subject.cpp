
#include "models/Subject.h"
#include <stdexcept>
#include <iostream>
using namespace std;

Subject::Subject(string id, string name, int credits)
    : SubjectID(id), SubjectName(name), Credits(credits) {

    if (credits < 0) {
        throw invalid_argument("So tin chi khong hop le");
    }
    if(id.size() != 5 || id.substr(0, 2) != "MH"){
        throw invalid_argument("Ma mon hoc phai co dang MHxxx");
    }
    for(int i = 2; i < 5; i++){
        if(id[i] < '0' || id[i] > '9'){
            throw invalid_argument("Ma mon hoc phai co dang MHxxx");
        }
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