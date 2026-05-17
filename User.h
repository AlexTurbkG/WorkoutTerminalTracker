#pragma once
#include "Workout.h"
#include "ProgressTracker.h"
#include "ExerciseCatalog.h"
#include <string>
#include <vector>
#include <map>

// ============================================================
// WORKOUT TEMPLATE
// ============================================================
struct WorkoutTemplate {
    std::string name;
    std::string notes;
    struct ExEntry { std::string exName; std::string muscleGroup; std::string type; };
    std::vector<ExEntry> exercises;
};

// ============================================================
// USER
// ============================================================
class User {
private:
    std::string name;
    int age;
    double weight;
    std::vector<Workout*> workouts;
    std::map<std::string, PersonalRecord> prs;
    std::map<std::string, double> bodyWeightLog;
    std::vector<WorkoutTemplate> templates;
    ExerciseCatalog catalog;

public:
    User(const std::string& n, int a, double w);
    ~User();

    const std::string& getName() const;
    int getAge() const;
    double getWeight() const;
    void setWeight(double w);

    ExerciseCatalog& getCatalog();
    std::map<std::string, PersonalRecord>& getPRs();
    std::map<std::string, double>& getBWLog();
    std::vector<WorkoutTemplate>& getTemplates();
    const std::vector<Workout*>& getWorkouts() const;

    void addWorkout(Workout* w);
    bool deleteWorkout(int idx);
    Workout* getWorkout(int idx);
    int workoutCount() const;

    void listWorkouts(bool brief = true) const;
    void logBodyWeight(const std::string& date, double bw);
    void printProfile() const;

    void saveToFile();
    static User* loadFromFile();
};
