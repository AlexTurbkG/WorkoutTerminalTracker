#pragma once
#include "Workout.h"
#include <map>
#include <vector>
#include <string>

// ============================================================
// PERSONAL RECORD
// ============================================================
struct PersonalRecord {
    double bestWeight;
    int bestReps;
    double bestVolume;
    double best1RM;
    std::string date;

    PersonalRecord()
        : bestWeight(0), bestReps(0), bestVolume(0), best1RM(0), date("") {}
};

// ============================================================
// PROGRESS TRACKER
// ============================================================
class ProgressTracker {
public:
    static void updatePRs(const Workout& w, std::map<std::string, PersonalRecord>& prs);
    static void printAllPRs(const std::map<std::string, PersonalRecord>& prs);
    static void weeklySummary(const std::vector<Workout*>& workouts, const std::string& weekStart);
    static void volumeProgressChart(const std::vector<Workout*>& workouts, const std::string& exerciseName);
    static void bodyWeightChart(const std::map<std::string, double>& bwLog);
};
