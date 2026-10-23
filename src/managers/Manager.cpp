#include "managers/Manager.h"

#include "models/Student.h"
#include "models/Teacher.h"
#include "models/Subject.h"
#include "models/CourseSection.h"
#include "models/Enrollment.h"
#include "models/Account.h"

// Destructor
template <typename T>
Manager<T>::~Manager() {
    for (auto& pair : this->List) {
        delete pair.second;
    }

    this->List.clear();
}

// Add
template <typename T>
bool Manager<T>::Add(T* item) {
    if (item == nullptr) {
        return false;
    }

    string id = item->GetID();

    if (this->List.find(id) != this->List.end()) {
        return false;
    }

    this->List[id] = item;
    return true;
}

// Remove
template <typename T>
bool Manager<T>::Remove(T* item) {
    if (item == nullptr) {
        return false;
    }

    auto it = this->List.find(item->GetID());

    if (it == this->List.end() || it->second != item) {
        return false;
    }

    delete it->second;
    this->List.erase(it);

    return true;
}

// Update
template <typename T>
bool Manager<T>::Update(const T& item) {
    T* oldItem = this->FindByID(item.GetID());

    if (oldItem == nullptr) {
        return false;
    }

    *oldItem = item;
    return true;
}

// FindByID
template <typename T>
T* Manager<T>::FindByID(const string& id) const {
    auto it = this->List.find(id);

    if (it == this->List.end()) {
        return nullptr;
    }

    return it->second;
}

// PrintByID
template <typename T>
void Manager<T>::PrintByID(const string& id) const {
    T* item = this->FindByID(id);

    if (item == nullptr) {
        cout << "ID not found!" << endl;
        return;
    }

    item->DisplayInfo();
}

// PrintAll
template <typename T>
void Manager<T>::PrintAll() const {
    if (this->List.empty()) {
        cout << "List is empty!" << endl;
        return;
    }

    for (const auto& pair : this->List) {
        pair.second->DisplayInfo();
        cout << "--------------------" << endl;
    }
}

// Explicit template instantiation
template class Manager<Student>;
template class Manager<Teacher>;
template class Manager<Subject>;
template class Manager<CourseSection>;
template class Manager<Enrollment>;
template class Manager<Account>;