#include "Workout.h"
#include <iostream>

Workout::Workout(const std::string& d, int dur, const std::string& n, const std::string& m)
    : date(d), durationMinutes(dur), notes(n), mood(m) {}

Workout::~Workout() {
    for (auto e : exercises) delete e;
}

void Workout::addExercise(Exercise* e) { exercises.push_back(e); }

const std::string& Workout::getDate() const { return date; }
int Workout::getDuration() const { return durationMinutes; }
const std::string& Workout::getNotes() const { return notes; }
const std::string& Workout::getMood() const { return mood; }
const std::vector<Exercise*>& Workout::getExercises() const { return exercises; }

void Workout::setDate(const std::string& d) { date = d; }
void Workout::setDuration(int d) { durationMinutes = d; }
void Workout::setNotes(const std::string& n) { notes = n; }
void Workout::setMood(const std::string& m) { mood = m; }

double Workout::getTotalVolume() const {
    double v = 0;
    for (const auto* e : exercises) v += e->getTotalVolume();
    return v;
}

int Workout::getTotalSets() const {
    int s = 0;
    for (const auto* e : exercises) s += (int)e->getSets().size();
    return s;
}

void Workout::print(bool brief) const {
    std::cout << "=== Workout: " << date << " | " << durationMinutes
              << " min | Mood: " << mood;
    if (!notes.empty()) std::cout << " | Notes: " << notes;
    std::cout << " ===\n";
    if (brief) {
        std::cout << "  " << exercises.size() << " exercises | "
                  << getTotalSets() << " sets | Volume: "
                  << getTotalVolume() << " kg\n";
        return;
    }
    for (const auto* e : exercises) {
        e->printDetails();
        std::cout << "\n";
    }
    std::cout << "  >> Total Volume: " << getTotalVolume() << " kg\n";
}
