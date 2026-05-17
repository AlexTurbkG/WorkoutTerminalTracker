#include "Menus.h"
#include "ProgressTracker.h"
#include <iostream>
#include <iomanip>
#include <map>

// ============================================================
// INPUT HELPERS
// ============================================================
std::string inputLine(const std::string& prompt) {
    std::cout << prompt;
    std::string s;
    std::getline(std::cin, s);
    return s;
}

int inputInt(const std::string& prompt) {
    while (true) {
        std::cout << prompt;
        std::string s;
        std::getline(std::cin, s);
        try { return std::stoi(s); }
        catch (...) { std::cout << "  >> Invalid number. Try again.\n"; }
    }
}

double inputDouble(const std::string& prompt) {
    while (true) {
        std::cout << prompt;
        std::string s;
        std::getline(std::cin, s);
        try { return std::stod(s); }
        catch (...) { std::cout << "  >> Invalid number. Try again.\n"; }
    }
}

// ============================================================
// ADD EXERCISE
// ============================================================
void menuAddExercise(Workout* workout, ExerciseCatalog& catalog) {
    while (true) {
        std::cout << "\n  -- Add Exercise --\n";
        std::cout << "  1. Pick from catalog\n";
        std::cout << "  2. Custom exercise\n";
        std::cout << "  0. Done adding exercises\n";
        int ch = inputInt("  > ");
        if (ch == 0) break;

        std::string exName, exType, exMG, exEq, exDesc;

        if (ch == 1) {
            catalog.listAll();
            exName = inputLine("  Exercise name (exact): ");
            if (!catalog.find(exName, exType, exMG, exEq, exDesc)) {
                std::cout << "  >> Not found in catalog.\n"; continue;
            }
        } else {
            exName = inputLine("  Name: ");
            exType = inputLine("  Type (Strength/Cardio): ");
            exMG   = inputLine("  Muscle group: ");
            exEq   = inputLine("  Equipment (or leave blank): ");
            exDesc = inputLine("  Description (or leave blank): ");
        }

        Exercise* ex = nullptr;
        if (exType == "Cardio") {
            double dist = inputDouble("  Distance (km, 0 if N/A): ");
            double pace = inputDouble("  Pace (min/km, 0 if N/A): ");
            ex = new CardioExercise(exName, exMG, dist, pace, exDesc);
        } else {
            if (exEq.empty()) exEq = "barbell";
            ex = new StrengthExercise(exName, exMG, exEq, exDesc);
        }

        std::cout << "  Add sets for " << exName << ":\n";
        while (true) {
            std::cout << "    (s)et | (d)one: ";
            std::string cmd;
            std::getline(std::cin, cmd);
            if (cmd == "d" || cmd == "done") break;
            if (cmd == "s" || cmd == "set") {
                try {
                    double w  = inputDouble("    Weight (kg): ");
                    int    r  = inputInt("    Reps: ");
                    int    rs = inputInt("    Rest (sec, default 60): ");
                    ex->addSet(Set(w, r, rs));
                    std::cout << "    >> Set added.\n";
                } catch (const std::exception& e) {
                    std::cout << "    >> Error: " << e.what() << "\n";
                }
            }
        }

        workout->addExercise(ex);
        std::cout << "  >> Exercise added.\n";
    }
}

// ============================================================
// NEW WORKOUT
// ============================================================
void menuNewWorkout(User& user) {
    std::cout << "\n-- New Workout --\n";
    std::string date  = inputLine("  Date (YYYY-MM-DD): ");
    int dur           = inputInt("  Duration (min): ");
    std::string notes = inputLine("  Notes (optional): ");
    std::string mood  = inputLine("  Mood (e.g. Great/Good/Average/Bad): ");
    if (mood.empty()) mood = "Good";

    Workout* w = new Workout(date, dur, notes, mood);
    menuAddExercise(w, user.getCatalog());
    user.addWorkout(w);
    std::cout << ">> Workout saved!\n";
}

// ============================================================
// EDIT PRs
// ============================================================
void menuEditPRs(User& user) {
    auto& prs = user.getPRs();
    if (prs.empty()) { std::cout << "  No PRs to edit.\n"; return; }

    ProgressTracker::printAllPRs(prs);
    std::string exName = inputLine("  Enter exercise name to edit PR (or blank to cancel): ");
    if (exName.empty()) return;

    if (prs.find(exName) == prs.end())
        std::cout << "  >> No PR for that exercise. Creating new entry.\n";

    auto& pr = prs[exName];
    std::cout << "  Leave field blank to keep current value.\n";
    std::string s;

    s = inputLine("  Best weight (kg) [current: " + std::to_string(pr.bestWeight) + "]: ");
    if (!s.empty()) pr.bestWeight = std::stod(s);

    s = inputLine("  Best reps [current: " + std::to_string(pr.bestReps) + "]: ");
    if (!s.empty()) pr.bestReps = std::stoi(s);

    s = inputLine("  Best volume (kg) [current: " + std::to_string(pr.bestVolume) + "]: ");
    if (!s.empty()) pr.bestVolume = std::stod(s);

    s = inputLine("  Best 1RM (kg) [current: " + std::to_string(pr.best1RM) + "]: ");
    if (!s.empty()) pr.best1RM = std::stod(s);

    s = inputLine("  Date (YYYY-MM-DD) [current: " + pr.date + "]: ");
    if (!s.empty()) pr.date = s;

    std::cout << "  >> PR updated.\n";
}

// ============================================================
// 1RM CALCULATOR
// ============================================================
void menuCalc1RM() {
    std::cout << "\n-- 1RM Calculator (Epley Formula) --\n";
    double w = inputDouble("  Weight used (kg): ");
    int    r = inputInt("  Reps performed: ");
    if (r <= 0) { std::cout << "  >> Invalid reps.\n"; return; }
    double oneRM = (r == 1) ? w : w * (1.0 + r / 30.0);
    std::cout << "  Estimated 1RM: " << std::fixed << std::setprecision(1) << oneRM << " kg\n";
    std::cout << "  Training percentages:\n";
    int percents[] = {100, 90, 85, 80, 75, 70, 65, 60};
    for (int p : percents)
        std::cout << "    " << p << "% -> " << std::fixed << std::setprecision(1)
                  << (oneRM * p / 100.0) << " kg\n";
}

// ============================================================
// TEMPLATES
// ============================================================
void menuTemplates(User& user) {
    auto& templates = user.getTemplates();
    while (true) {
        std::cout << "\n-- Workout Templates --\n";
        std::cout << "  1. View templates\n";
        std::cout << "  2. Create template\n";
        std::cout << "  3. Delete template\n";
        std::cout << "  0. Back\n";
        int ch = inputInt("  > ");
        if (ch == 0) break;

        if (ch == 1) {
            if (templates.empty()) { std::cout << "  No templates saved.\n"; continue; }
            for (int i = 0; i < (int)templates.size(); i++) {
                std::cout << "  [" << i << "] " << templates[i].name << "\n";
                if (!templates[i].notes.empty())
                    std::cout << "      Notes: " << templates[i].notes << "\n";
                for (const auto& ex : templates[i].exercises)
                    std::cout << "      - " << ex.exName << " [" << ex.muscleGroup << "]\n";
            }
        } else if (ch == 2) {
            WorkoutTemplate t;
            t.name  = inputLine("  Template name: ");
            t.notes = inputLine("  Notes (optional): ");
            std::cout << "  Add exercises (blank name to stop):\n";
            while (true) {
                std::string n = inputLine("    Exercise name: ");
                if (n.empty()) break;
                std::string mg   = inputLine("    Muscle group: ");
                std::string type = inputLine("    Type (Strength/Cardio): ");
                t.exercises.push_back({n, mg, type});
            }
            templates.push_back(t);
            std::cout << "  >> Template saved.\n";
        } else if (ch == 3) {
            if (templates.empty()) { std::cout << "  No templates.\n"; continue; }
            int idx = inputInt("  Template index to delete: ");
            if (idx >= 0 && idx < (int)templates.size()) {
                templates.erase(templates.begin() + idx);
                std::cout << "  >> Deleted.\n";
            } else std::cout << "  >> Invalid index.\n";
        }
    }
}

// ============================================================
// BODY WEIGHT
// ============================================================
void menuBodyWeight(User& user) {
    while (true) {
        std::cout << "\n-- Body Weight Tracker --\n";
        std::cout << "  1. Log weight\n";
        std::cout << "  2. View progress chart\n";
        std::cout << "  0. Back\n";
        int ch = inputInt("  > ");
        if (ch == 0) break;
        if (ch == 1) {
            std::string date = inputLine("  Date (YYYY-MM-DD): ");
            double bw        = inputDouble("  Weight (kg): ");
            user.logBodyWeight(date, bw);
            std::cout << "  >> Logged.\n";
        } else if (ch == 2) {
            ProgressTracker::bodyWeightChart(user.getBWLog());
        }
    }
}

// ============================================================
// MUSCLE FREQUENCY
// ============================================================
void menuMuscleFrequency(const User& user) {
    std::map<std::string, int> freq;
    for (const auto* w : user.getWorkouts())
        for (const auto* ex : w->getExercises())
            freq[ex->getMuscleGroup()]++;

    std::cout << "\n-- Muscle Group Frequency --\n";
    if (freq.empty()) { std::cout << "  No data.\n\n"; return; }

    int maxFreq = 0;
    for (const auto& kv : freq) maxFreq = std::max(maxFreq, kv.second);
    int barW = 20;
    for (const auto& kv : freq) {
        int bars = (maxFreq > 0) ? (int)((double)kv.second / maxFreq * barW) : 0;
        std::cout << "  " << std::left << std::setw(14) << kv.first << " | ";
        for (int i = 0; i < bars; i++) std::cout << "\xe2\x96\xa0";
        for (int i = bars; i < barW; i++) std::cout << "\xc2\xb7";
        std::cout << " " << kv.second << "x\n";
    }
    std::cout << "\n";
}
