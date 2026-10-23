
#pragma once
#include <string>

class Subject {
    private:
        std::string SubjectID;
        std::string SubjectName;
        int Credits;

    public:
        Subject(std::string = "", std::string = "", int = 0);

        std::string GetID() const;
        std::string GetSubjectName() const;
        int GetCredits() const;

        void SetSubjectName(const std::string& name);
        void SetCredits(int credits);
        void DisplayInfo() const;
};
