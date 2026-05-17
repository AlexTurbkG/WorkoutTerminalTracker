#pragma once
#include "Exercise.h"
#include <string>
#include <vector>

class Workout {
private:
    std::string date;
    int durationMinutes;
    std::string notes;
    std::vector<Exercise*> exercises;
    std::string mood;

public:
    Workout(const std::string& d, int dur, const std::string& n = "", const std::string& m = "Good");
    ~Workout();

    // No copy
    Workout(const Workout&) = delete;
    Workout& operator=(const Workout&) = delete;

    void addExercise(Exercise* e);

    const std::string& getDate() const;
    int getDuration() const;
    const std::string& getNotes() const;
    const std::string& getMood() const;
    const std::vector<Exercise*>& getExercises() const;

    void setDate(const std::string& d);
    void setDuration(int d);
    void setNotes(const std::string& n);
    void setMood(const std::string& m);

    double getTotalVolume() const;
    int getTotalSets() const;

    void print(bool brief = false) const;
};
