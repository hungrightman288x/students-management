#pragma once

#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include "managers/Manager.h"
#include "storage/RecordIO.h"

template<class T>
class PersistentManager : public Manager<T> {
    std::string FileName;

public:
    explicit PersistentManager(const std::string& fileName) : FileName(fileName) { LoadFromFile(); }

    // Parse into a temporary list: malformed files never partially replace data.
    void LoadFromFile() {
        if (FileName.empty() || !std::filesystem::exists(FileName)) return;
        std::ifstream input(FileName);
        if (!input) throw std::runtime_error("Khong mo duoc file: " + FileName);
        decltype(this->List) parsed;
        std::string line;
        std::size_t number = 0;
        while (std::getline(input, line)) {
            ++number;
            if (number == 1 && line.compare(0, 3, "\xEF\xBB\xBF") == 0) line.erase(0, 3);
            auto start = line.find_first_not_of(" \t\r");
            if (start == std::string::npos || line[start] == '#') continue;
            try {
                std::istringstream row(line);
                auto item = std::make_unique<T>();
                ReadRecord(row, *item);
                if (!row) throw std::invalid_argument("Thieu truong hoac sai kieu du lieu");
                row >> std::ws;
                if (!row.eof()) throw std::invalid_argument("Du truong du lieu");
                ValidateText(item->GetID(), "Ma");
                if (parsed.count(item->GetID())) throw std::invalid_argument("Trung ma");
                const auto id = item->GetID();
                parsed.emplace(id, std::move(item));
            } catch (const std::exception& error) {
                throw std::runtime_error(FileName + ":" + std::to_string(number) + ": " + error.what());
            }
        }
        if (input.bad()) throw std::runtime_error("Loi doc file: " + FileName);
        this->List.swap(parsed);
    }

    void SaveToFile() const { SaveToFile(FileName); }

    void SaveToFile(const std::string& fileName) const {
        if (fileName.empty()) throw std::runtime_error("Chua dat ten file du lieu");
        std::filesystem::path target(fileName);
        if (target.has_parent_path()) std::filesystem::create_directories(target.parent_path());
        auto temporary = target;
        temporary += ".tmp";
        if (std::filesystem::exists(temporary))
            throw std::runtime_error("Duong dan tam da ton tai: " + temporary.string());
        try {
            std::ofstream output(temporary, std::ios::trunc);
            if (!output) throw std::runtime_error("Khong ghi duoc file: " + temporary.string());
            output << "# student-management v1\n";
            output << std::setprecision(std::numeric_limits<float>::max_digits10);
            for (auto item : this->GetAll()) {
                WriteRecord(output, *item);
                output << '\n';
            }
            output.flush();
            if (!output) throw std::runtime_error("Loi ghi file: " + temporary.string());
            output.close();
            if (!output) throw std::runtime_error("Loi dong file: " + temporary.string());
            AtomicReplaceFile(temporary, target);
        } catch (...) {
            std::error_code ignored;
            std::filesystem::remove(temporary, ignored);
            throw;
        }
    }
};
