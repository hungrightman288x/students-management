
#include "models/Subject.h"
#include <stdexcept>
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
