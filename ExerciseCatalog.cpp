#include "ExerciseCatalog.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

ExerciseCatalog::ExerciseCatalog() {
    entries = {
        {"Bench Press",      "Chest",      "Strength", "barbell",    "Compound push movement"},
        {"Incline DB Press", "Chest",      "Strength", "dumbbell",   "Upper chest focus"},
        {"Cable Fly",        "Chest",      "Strength", "machine",    "Isolation movement"},
        {"Squat",            "Legs",       "Strength", "barbell",    "King of leg exercises"},
        {"Leg Press",        "Legs",       "Strength", "machine",    "Quad dominant"},
        {"Romanian DL",      "Hamstrings", "Strength", "barbell",    "Hip hinge pattern"},
        {"Deadlift",         "Back",       "Strength", "barbell",    "Full body compound"},
        {"Pull-up",          "Back",       "Strength", "bodyweight", "Vertical pull"},
        {"Barbell Row",      "Back",       "Strength", "barbell",    "Horizontal pull"},
        {"Overhead Press",   "Shoulders",  "Strength", "barbell",    "Vertical push"},
        {"Lateral Raise",    "Shoulders",  "Strength", "dumbbell",   "Medial delt isolation"},
        {"Bicep Curl",       "Biceps",     "Strength", "dumbbell",   "Elbow flexion"},
        {"Tricep Pushdown",  "Triceps",    "Strength", "machine",    "Elbow extension"},
        {"Plank",            "Core",       "Strength", "bodyweight", "Anti-extension"},
        {"Running",          "Cardio",     "Cardio",   "",           "Steady state cardio"},
        {"Cycling",          "Cardio",     "Cardio",   "",           "Low impact cardio"},
        {"Jump Rope",        "Cardio",     "Cardio",   "",           "High intensity cardio"},
    };
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
