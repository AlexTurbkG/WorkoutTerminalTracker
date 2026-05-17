#include "Exercise.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

// ============================================================
// EXERCISE
// ============================================================
Exercise::Exercise(const std::string& n, const std::string& mg, const std::string& desc)
    : name(n), muscleGroup(mg), description(desc) {}

const std::string& Exercise::getName() const { return name; }
const std::string& Exercise::getMuscleGroup() const { return muscleGroup; }
const std::string& Exercise::getDescription() const { return description; }
const std::vector<Set>& Exercise::getSets() const { return sets; }

void Exercise::setName(const std::string& n) { name = n; }
void Exercise::setDescription(const std::string& d) { description = d; }
void Exercise::addSet(const Set& s) { sets.push_back(s); }

double Exercise::getTotalVolume() const {
    double vol = 0;
    for (const auto& s : sets) vol += s.getVolume();
    return vol;
}

double Exercise::getBest1RM() const {
    double best = 0;
    for (const auto& s : sets)
        if (s.calc1RM() > best) best = s.calc1RM();
    return best;
}

double Exercise::getMaxWeight() const {
    double best = 0;
    for (const auto& s : sets)
        if (s.getWeight() > best) best = s.getWeight();
    return best;
}

int Exercise::getMaxReps() const {
    int best = 0;
    for (const auto& s : sets)
        if (s.getReps() > best) best = s.getReps();
    return best;
}

void Exercise::printSets() const {
    if (sets.empty()) { std::cout << "    (no sets)\n"; return; }
    for (int i = 0; i < (int)sets.size(); i++)
        sets[i].print(i + 1);
}

// ============================================================
// STRENGTH EXERCISE
// ============================================================
StrengthExercise::StrengthExercise(const std::string& n, const std::string& mg,
                                   const std::string& eq, const std::string& desc)
    : Exercise(n, mg, desc), equipment(eq) {}

std::string StrengthExercise::getType() const { return "Strength"; }
const std::string& StrengthExercise::getEquipment() const { return equipment; }

void StrengthExercise::printDetails() const {
    std::cout << "[STRENGTH] " << name << " | " << muscleGroup
              << " | " << equipment << "\n";
    if (!description.empty()) std::cout << "  Info: " << description << "\n";
    printSets();
    if (!sets.empty())
        std::cout << "  Total volume: " << getTotalVolume() << " kg\n";
}

// ============================================================
// CARDIO EXERCISE
// ============================================================
CardioExercise::CardioExercise(const std::string& n, const std::string& mg,
                               double dist, double pace, const std::string& desc)
    : Exercise(n, mg, desc), distanceKm(dist), paceMinPerKm(pace) {}

std::string CardioExercise::getType() const { return "Cardio"; }
double CardioExercise::getDistance() const { return distanceKm; }
double CardioExercise::getPace() const { return paceMinPerKm; }
void CardioExercise::setDistance(double d) { distanceKm = d; }
void CardioExercise::setPace(double p) { paceMinPerKm = p; }

void CardioExercise::printDetails() const {
    std::cout << "[CARDIO] " << name << " | " << muscleGroup << "\n";
    if (distanceKm > 0)
        std::cout << "  Distance: " << distanceKm << " km  Pace: "
                  << paceMinPerKm << " min/km\n";
    if (!description.empty()) std::cout << "  Info: " << description << "\n";
    printSets();
}
