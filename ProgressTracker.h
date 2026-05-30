#pragma once
#include "Workout.h"
#include <map>
#include <vector>
#include <string>

// ============================================================
// PERSONAL RECORD
// ============================================================
struct PersonalRecord {
    double      bestWeight = 0;
    int         bestReps   = 0;
    double      bestVolume = 0;
    double      best1RM    = 0;
    std::string date;
};

// ============================================================
// STRENGTH STANDARD  (relative to bodyweight)
// ============================================================
struct StrengthStandard {
    std::string exercise;
    // 1RM as a multiple of bodyweight for each level
    double beginner;
    double intermediate;
    double advanced;
    double elite;
};

// ============================================================
// PROGRESS TRACKER
// ============================================================
class ProgressTracker {
public:
    // ── Core PR tracking ─────────────────────────────────────
    static void updatePRs(const Workout& w,
                          std::map<std::string, PersonalRecord>& prs);
    static void printAllPRs(const std::map<std::string, PersonalRecord>& prs);

    // ── Charts ───────────────────────────────────────────────
    static void volumeProgressChart(const std::vector<Workout*>& workouts,
                                    const std::string& exerciseName);
    static void muscleVolumeChart(const std::vector<Workout*>& workouts);
    static void bodyWeightChart(const std::map<std::string, double>& bwLog);
    static void workoutHeatmap(const std::vector<Workout*>& workouts);

    // ── Summaries ────────────────────────────────────────────
    static void weeklySummary(const std::vector<Workout*>& workouts,
                              const std::string& weekStart);

    // ── Strength standards ───────────────────────────────────
    static void strengthStandards(
        const std::map<std::string, PersonalRecord>& prs,
        double bodyweightKg);
};