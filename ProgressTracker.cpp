#include "ProgressTracker.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

void ProgressTracker::updatePRs(const Workout& w, std::map<std::string, PersonalRecord>& prs) {
    for (const auto* ex : w.getExercises()) {
        auto& pr = prs[ex->getName()];
        const std::string& date = w.getDate();
        bool updated = false;

        for (const auto& s : ex->getSets()) {
            if (s.getWeight() > pr.bestWeight) { pr.bestWeight = s.getWeight(); updated = true; }
            if (s.getReps() > pr.bestReps)     { pr.bestReps = s.getReps();     updated = true; }
            if (s.getVolume() > pr.bestVolume) { pr.bestVolume = s.getVolume(); updated = true; }
            if (s.calc1RM() > pr.best1RM)      { pr.best1RM = s.calc1RM();     updated = true; }
        }
        if (updated) pr.date = date;
    }
}

void ProgressTracker::printAllPRs(const std::map<std::string, PersonalRecord>& prs) {
    if (prs.empty()) { std::cout << "  No personal records yet.\n"; return; }
    std::cout << "\n--- Personal Records ---\n";
    std::cout << std::left
              << std::setw(22) << "Exercise"
              << std::setw(12) << "Max Weight"
              << std::setw(10) << "Max Reps"
              << std::setw(14) << "Max Volume"
              << std::setw(12) << "Est. 1RM"
              << "Date\n";
    std::cout << std::string(80, '-') << "\n";
    for (const auto& kv : prs) {
        const auto& pr = kv.second;
        std::cout << std::left
                  << std::setw(22) << kv.first
                  << std::setw(12) << (std::to_string((int)pr.bestWeight) + " kg")
                  << std::setw(10) << pr.bestReps
                  << std::setw(14) << (std::to_string((int)pr.bestVolume) + " kg")
                  << std::setw(12) << (std::to_string((int)pr.best1RM) + " kg")
                  << pr.date << "\n";
    }
    std::cout << "\n";
}

void ProgressTracker::weeklySummary(const std::vector<Workout*>& workouts, const std::string& weekStart) {
    std::cout << "\n--- Weekly Summary (from " << weekStart << ") ---\n";
    int count = 0, totalSets = 0, totalDuration = 0;
    double totalVolume = 0;
    std::map<std::string, int> muscleCount;

    for (const auto* w : workouts) {
        if (w->getDate() >= weekStart) {
            count++;
            totalVolume   += w->getTotalVolume();
            totalSets     += w->getTotalSets();
            totalDuration += w->getDuration();
            for (const auto* ex : w->getExercises())
                muscleCount[ex->getMuscleGroup()]++;
        }
    }

    std::cout << "  Workouts completed : " << count << "\n";
    std::cout << "  Total duration     : " << totalDuration << " min\n";
    std::cout << "  Total sets         : " << totalSets << "\n";
    std::cout << "  Total volume       : " << totalVolume << " kg\n";
    if (!muscleCount.empty()) {
        std::cout << "  Muscle groups hit  : ";
        for (const auto& kv : muscleCount)
            std::cout << kv.first << "(" << kv.second << ") ";
        std::cout << "\n";
    }
    std::cout << "\n";
}

void ProgressTracker::volumeProgressChart(const std::vector<Workout*>& workouts,
                                          const std::string& exerciseName) {
    std::cout << "\n--- Volume Progress: " << exerciseName << " ---\n";
    std::vector<std::pair<std::string, double>> data;
    for (const auto* w : workouts)
        for (const auto* ex : w->getExercises())
            if (ex->getName() == exerciseName && !ex->getSets().empty())
                data.push_back({w->getDate(), ex->getTotalVolume()});

    if (data.empty()) { std::cout << "  No data for this exercise.\n\n"; return; }

    double maxVol = 0;
    for (const auto& d : data) maxVol = std::max(maxVol, d.second);
    int barWidth = 30;
    for (const auto& kv : data) {
        int bars = (maxVol > 0) ? (int)((kv.second / maxVol) * barWidth) : 0;
        std::cout << "  " << kv.first << " | ";
        for (int i = 0; i < bars; i++) std::cout << "\xe2\x96\x88";
        for (int i = bars; i < barWidth; i++) std::cout << "\xe2\x96\x91";
        std::cout << " " << std::fixed << std::setprecision(0) << kv.second << " kg\n";
    }
    std::cout << "\n";
}

void ProgressTracker::bodyWeightChart(const std::map<std::string, double>& bwLog) {
    if (bwLog.empty()) { std::cout << "  No body weight data.\n"; return; }
    std::cout << "\n--- Body Weight Progress ---\n";
    double minW = 1e9, maxW = 0;
    for (const auto& kv : bwLog) { minW = std::min(minW, kv.second); maxW = std::max(maxW, kv.second); }
    double range = maxW - minW;
    int barWidth = 25;
    for (const auto& kv : bwLog) {
        int bars = (range > 0) ? (int)(((kv.second - minW) / range) * barWidth) : barWidth / 2;
        std::cout << "  " << kv.first << " | ";
        for (int i = 0; i < bars; i++) std::cout << "\xe2\x96\x93";
        for (int i = bars; i < barWidth; i++) std::cout << "\xe2\x96\x91";
        std::cout << " " << kv.second << " kg\n";
    }
    std::cout << "\n";
}
