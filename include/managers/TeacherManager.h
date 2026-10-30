#pragma once
#include "models/Teacher.h"
#include "managers/Manager.h"
#include <string>
using namespace std;
class TeacherManager : public Manager<Teacher>{
    private:
        string FileName;
        void LoadFromFile();
        public:
        void SaveToFile() const;
        TeacherManager(const string&);
        ~TeacherManager();
};
