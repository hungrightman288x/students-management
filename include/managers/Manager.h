#pragma once

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// Owns accepted objects. On a failed Add(T*), ownership stays with the caller.
template <typename T>
class Manager {
protected:
    std::unordered_map<std::string, std::unique_ptr<T>> List;

public:
    Manager() = default;
    virtual ~Manager() = default;
    Manager(const Manager&) = delete;
    Manager& operator=(const Manager&) = delete;

    bool Add(T* item) {
        if (!item || item->GetID().empty() || List.count(item->GetID())) return false;
        auto result = List.try_emplace(item->GetID(), nullptr);
        result.first->second.reset(item);
        return true;
    }

    bool Add(std::unique_ptr<T> item) {
        if (!Add(item.get())) return false;
        item.release();
        return true;
    }

    bool Remove(T* item) {
        if (!item) return false;
        auto found = List.find(item->GetID());
        if (found == List.end() || found->second.get() != item) return false;
        List.erase(found);
        return true;
    }

    bool Update(const T& item) {
        auto old = FindByID(item.GetID());
        if (!old) return false;
        *old = item;
        return true;
    }

    T* FindByID(const std::string& id) {
        auto found = List.find(id);
        return found == List.end() ? nullptr : found->second.get();
    }

    const T* FindByID(const std::string& id) const {
        auto found = List.find(id);
        return found == List.end() ? nullptr : found->second.get();
    }

    std::vector<const T*> GetAll() const {
        std::vector<const T*> result;
        for (const auto& pair : List) result.push_back(pair.second.get());
        std::sort(result.begin(), result.end(), [](const T* a, const T* b) {
            return a->GetID() < b->GetID();
        });
        return result;
    }

    std::size_t Size() const { return List.size(); }
    bool Empty() const { return List.empty(); }

    void PrintByID(const std::string& id) const {
        auto item = FindByID(id);
        if (item) item->DisplayInfo();
        else std::cout << "Khong tim thay ma!\n";
    }

    void PrintAll() const {
        if (Empty()) std::cout << "Danh sach rong.\n";
        for (auto item : GetAll()) {
            item->DisplayInfo();
            std::cout << "--------------------\n";
        }
    }
};
