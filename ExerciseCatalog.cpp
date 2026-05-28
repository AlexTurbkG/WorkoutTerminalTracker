#include "ExerciseCatalog.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <fstream>
#include "nlohmann/json.hpp"
using json = nlohmann::json;

ExerciseCatalog::ExerciseCatalog() {
    std::ifstream file("exercises.json");
    if (!file.is_open()) {
        std::cerr << "  Warning: Could not open exercises.json. Starting with empty catalog.\n";
        return;
    }
    try {
        json j;
        file >> j;
        for (const auto& item : j) {
            entries.push_back({
                item.value("name",        "Unknown"),
                item.value("muscleGroup", "Unknown"),
                item.value("type",        "Strength"),
                item.value("equipment",   ""),
                item.value("description", "")
            });
        }
        // Keep entries sorted by name for consistent display
        std::sort(entries.begin(), entries.end(),
            [](const Entry& a, const Entry& b){ return a.name < b.name; });
    } catch (const json::parse_error& e) {
        std::cerr << "  JSON Parse Error: " << e.what() << "\n";
    }
}

void ExerciseCatalog::addEntry(const std::string& name, const std::string& mg,
                               const std::string& type, const std::string& eq,
                               const std::string& desc) {
    entries.push_back({name, mg, type, eq, desc});
}

void ExerciseCatalog::listAll() const {
    std::cout << "\n  --- Exercise Catalog ---\n";
    auto sorted = entries;
    std::sort(sorted.begin(), sorted.end(),
        [](const Entry& a, const Entry& b){ return a.muscleGroup < b.muscleGroup; });
    std::string lastMG;
    for (const auto& e : sorted) {
        if (e.muscleGroup != lastMG) {
            std::cout << "\n  \xe2\x96\xb8 " << e.muscleGroup << "\n";
            lastMG = e.muscleGroup;
        }
        std::cout << "    " << std::left << std::setw(34) << e.name
                  << std::setw(12) << e.type
                  << e.equipment << "\n";
    }
    std::cout << "\n";
}

void ExerciseCatalog::listByMuscle(const std::string& mg) const {
    std::cout << "\n  --- " << mg << " ---\n";
    bool found = false;
    for (const auto& e : entries) {
        if (e.muscleGroup == mg) {
            std::cout << "    " << e.name << "  [" << e.type << "]  " << e.equipment << "\n";
            found = true;
        }
    }
    if (!found) std::cout << "  (none found)\n";
    std::cout << "\n";
}

bool ExerciseCatalog::find(const std::string& name, std::string& outType, std::string& outMG,
                           std::string& outEq, std::string& outDesc) const {
    for (const auto& e : entries) {
        if (e.name == name) {
            outType = e.type; outMG = e.muscleGroup;
            outEq = e.equipment; outDesc = e.description;
            return true;
        }
    }
    return false;
}