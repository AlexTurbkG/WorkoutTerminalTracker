#include "ExerciseCatalog.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <fstream>              // Required for file streams
#include "nlohmann/json.hpp"    // Required for JSON parsing
using json = nlohmann::json;

ExerciseCatalog::ExerciseCatalog() {
    // Instead of hardcoding, we attempt to load from the JSON file on initialization
    std::ifstream file("exercises.json");
    if (!file.is_open()) {
        std::cerr << "Warning: Could not open exercises.json. Starting with an empty catalog.\n";
        return;
    }

    try {
        json j;
        file >> j; // Parse the JSON file

        for (const auto& item : j) {
            // Map the JSON keys to your struct fields
            entries.push_back({
                item.value("name", "Unknown"),
                item.value("muscleGroup", "Unknown"),
                item.value("type", "Unknown"),
                item.value("equipment", ""),
                item.value("description", "")
            });
        }
    } catch (const json::parse_error& e) {
        std::cerr << "JSON Parse Error: " << e.what() << "\n";
    }
}

void ExerciseCatalog::addEntry(const std::string& name, const std::string& mg,
                               const std::string& type, const std::string& eq,
                               const std::string& desc) {
    entries.push_back({name, mg, type, eq, desc});
}

void ExerciseCatalog::listAll() const {
    std::cout << "\n--- Exercise Catalog ---\n";
    auto sorted = entries;
    std::sort(sorted.begin(), sorted.end(),
        [](const CatalogEntry& a, const CatalogEntry& b){ return a.muscleGroup < b.muscleGroup; });
    std::string lastMG;
    for (const auto& e : sorted) {
        if (e.muscleGroup != lastMG) {
            std::cout << "\n[" << e.muscleGroup << "]\n";
            lastMG = e.muscleGroup;
        }
        std::cout << "  " << std::left << std::setw(20) << e.name
                  << " | " << std::setw(10) << e.type
                  << " | " << e.equipment << "\n";
    }
    std::cout << "\n";
}

void ExerciseCatalog::listByMuscle(const std::string& mg) const {
    std::cout << "\n--- Exercises for: " << mg << " ---\n";
    bool found = false;
    for (const auto& e : entries) {
        if (e.muscleGroup == mg) {
            std::cout << "  " << e.name << " [" << e.type << "] " << e.equipment << "\n";
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