#pragma once
#include "managers/Manager.h"
#include "models/Student.h"
#include <iostream>
using namespace std;
class StudentManager : public Manager<Student>{
    private:
        string FileName;
        void LoadFromFile();
        public:
        void SaveToFile() const;
        StudentManager(const string&);
        ~StudentManager();
};