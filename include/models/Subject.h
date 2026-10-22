
#pragma once
#include <string>
using namespace std;

class Subject {
    private:
        string SubjectID;
        string SubjectName;
        int Credits;

    public:
        Subject(string = "", string = "", int = 0);

        string GetID() const;
        string GetSubjectName() const;
        int GetCredits() const;

        void SetSubjectName(const string& name);
};
