#ifndef MANAGER_H
#define MANAGER_H

#include <iostream>
#include <string>
#include <unordered_map>

using namespace std;

template <typename T>
class Manager {
protected:
    unordered_map<string, T*> List;

public:
    ~Manager();

    bool Add(T* item);
    bool Remove(T* item);
    bool Update(const T& item);

    T* FindByID(const string& id) const;

    void PrintByID(const string& id) const;
    void PrintAll() const;
    const unordered_map<string, T*>& GetAll() const {
        return this->List;
    }
};

#endif