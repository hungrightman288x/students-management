#pragma once
#include "models/Subject.h"
#include "managers/Manager.h"
#include <string>
using namespace std;
class SubjectManager : public Manager<Subject>{
    private:
        string FileName;
        void LoadFromFile();
        public:
        void SaveToFile() const;
        SubjectManager(const string&);
        ~SubjectManager();
};