#include "Menus.h"
#include "ProgressTracker.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <map>
#include <ctime>

// ============================================================
// ANSI COLOUR HELPERS  (Windows UTF-8 console supports these)
// ============================================================
namespace C {
    const char* RESET   = "\033[0m";
    const char* BOLD    = "\033[1m";
    const char* DIM     = "\033[2m";
    const char* GREEN   = "\033[32m";
    const char* YELLOW  = "\033[33m";
    const char* CYAN    = "\033[36m";
    const char* RED     = "\033[31m";
    const char* MAGENTA = "\033[35m";
    const char* BGREEN  = "\033[1;32m";
    const char* BCYAN   = "\033[1;36m";
    const char* BYELLOW = "\033[1;33m";
}

// ============================================================
// LAYOUT CONSTANTS
// ============================================================
static const int COL = 72;   // total usable column width

static void hr(char ch = '-') {
    std::cout << "  " << std::string(COL, ch) << "\n";
}

static void header(const std::string& title) {
    std::cout << "\n";
    hr('=');
    // Centre the title
    int pad = (COL - (int)title.size()) / 2;
    std::cout << "  " << std::string(std::max(pad, 0), ' ')
              << C::BOLD << C::CYAN << title << C::RESET << "\n";
    hr('=');
}

static void subheader(const std::string& title) {
    std::cout << "\n  " << C::BOLD << C::YELLOW << "\xe2\x96\xba " << title << C::RESET << "\n";
    hr();
}

static void ok  (const std::string& msg) { std::cout << "  " << C::BGREEN  << "\xe2\x9c\x94  " << msg << C::RESET << "\n"; }
static void warn(const std::string& msg) { std::cout << "  " << C::YELLOW  << "\xe2\x9a\xa0  " << msg << C::RESET << "\n"; }
static void err (const std::string& msg) { std::cout << "  " << C::RED     << "\xe2\x9c\x96  " << msg << C::RESET << "\n"; }
static void info(const std::string& msg) { std::cout << "  " << C::CYAN    << "\xe2\x84\xb9  " << msg << C::RESET << "\n"; }

// ============================================================
// INPUT HELPERS
// ============================================================
std::string inputLine(const std::string& prompt) {
    std::cout << C::BOLD << "  " << prompt << C::RESET;
    std::string s;
    std::getline(std::cin, s);
    // trim
    s.erase(0, s.find_first_not_of(" \t"));
    s.erase(s.find_last_not_of(" \t") + 1);
    return s;
}

int inputInt(const std::string& prompt) {
    while (true) {
        std::cout << C::BOLD << "  " << prompt << C::RESET;
        std::string s;
        std::getline(std::cin, s);
        s.erase(0, s.find_first_not_of(" \t"));
        s.erase(s.find_last_not_of(" \t") + 1);
        try {
            size_t pos;
            int v = std::stoi(s, &pos);
            if (pos == s.size()) return v;
        } catch (...) {}
        err("Please enter a whole number.");
    }
}

// inputInt with a default value shown in prompt, returned on blank entry
static int inputIntOr(const std::string& prompt, int def) {
    while (true) {
        std::cout << C::BOLD << "  " << prompt << C::DIM << " [" << def << "]" << C::RESET << ": ";
        std::string s;
        std::getline(std::cin, s);
        s.erase(0, s.find_first_not_of(" \t"));
        s.erase(s.find_last_not_of(" \t") + 1);
        if (s.empty()) return def;
        try {
            size_t pos;
            int v = std::stoi(s, &pos);
            if (pos == s.size()) return v;
        } catch (...) {}
        err("Please enter a whole number.");
    }
}

double inputDouble(const std::string& prompt) {
    while (true) {
        std::cout << C::BOLD << "  " << prompt << C::RESET;
        std::string s;
        std::getline(std::cin, s);
        s.erase(0, s.find_first_not_of(" \t"));
        s.erase(s.find_last_not_of(" \t") + 1);
        try {
            size_t pos;
            double v = std::stod(s, &pos);
            if (pos == s.size()) return v;
        } catch (...) {}
        err("Please enter a number (e.g. 60 or 72.5).");
    }
}

// Optional double — returns def on blank
static double inputDoubleOr(const std::string& prompt, double def) {
    while (true) {
        std::cout << C::BOLD << "  " << prompt << C::DIM << " [" << def << "]" << C::RESET << ": ";
        std::string s;
        std::getline(std::cin, s);
        s.erase(0, s.find_first_not_of(" \t"));
        s.erase(s.find_last_not_of(" \t") + 1);
        if (s.empty()) return def;
        try {
            size_t pos;
            double v = std::stod(s, &pos);
            if (pos == s.size()) return v;
        } catch (...) {}
        err("Please enter a number.");
    }
}

// Yes/no confirm — returns true on y/Y/yes, false on n/N/no or blank
static bool confirm(const std::string& question) {
    while (true) {
        std::cout << "  " << C::BOLD << question << C::RESET << C::DIM << " (y/n)" << C::RESET << " ";
        std::string s;
        std::getline(std::cin, s);
        if (s.empty() || s == "n" || s == "N") return false;
        if (s == "y" || s == "Y") return true;
        err("Please type y or n.");
    }
}

// Today's date as YYYY-MM-DD
static std::string todayDate() {
    time_t t = time(nullptr);
    char buf[11];
    strftime(buf, sizeof(buf), "%Y-%m-%d", localtime(&t));
    return std::string(buf);
}

// ============================================================
// EXERCISE BROWSER
// ============================================================
namespace Browser {

static const int PAGE = 14;

struct Filter {
    std::string search;
    std::string muscleGroup;
    std::string type;          // "" | "Strength" | "Cardio"
};

static bool icontains(const std::string& hay, const std::string& needle) {
    if (needle.empty()) return true;
    return std::search(hay.begin(), hay.end(), needle.begin(), needle.end(),
        [](char a, char b){ return std::tolower((unsigned char)a) ==
                                   std::tolower((unsigned char)b); }) != hay.end();
}

static std::vector<std::string> uniqueMuscles(const std::vector<ExerciseCatalog::Entry>& all) {
    std::vector<std::string> v;
    for (const auto& e : all)
        if (std::find(v.begin(), v.end(), e.muscleGroup) == v.end())
            v.push_back(e.muscleGroup);
    std::sort(v.begin(), v.end());
    return v;
}

static std::vector<const ExerciseCatalog::Entry*>
applyFilter(const std::vector<ExerciseCatalog::Entry>& all, const Filter& f) {
    std::vector<const ExerciseCatalog::Entry*> out;
    for (const auto& e : all) {
        if (!icontains(e.name, f.search))                   continue;
        if (!f.muscleGroup.empty() && e.muscleGroup != f.muscleGroup) continue;
        if (!f.type.empty()        && e.type        != f.type)        continue;
        out.push_back(&e);
    }
    return out;
}

static void printFilterBar(const Filter& f) {
    std::cout << "  " << C::DIM << "Filters: ";
    bool any = false;
    if (!f.search.empty())      { std::cout << "search=\"" << f.search << "\"  "; any = true; }
    if (!f.muscleGroup.empty()) { std::cout << "muscle=" << f.muscleGroup << "  "; any = true; }
    if (!f.type.empty())        { std::cout << "type=" << f.type; any = true; }
    if (!any) std::cout << "none";
    std::cout << C::RESET << "\n";
}

static void printPage(const std::vector<const ExerciseCatalog::Entry*>& view,
                      int page, int total) {
    int start = page * PAGE;
    int end   = std::min(start + PAGE, (int)view.size());

    // Table header
    std::cout << "\n  " << C::BOLD
              << std::left
              << std::setw(4)  << "#"
              << std::setw(38) << "Exercise"
              << std::setw(14) << "Muscle"
              << std::setw(12) << "Equipment"
              << std::setw(10) << "Type"
              << C::RESET << "\n";
    hr();

    for (int i = start; i < end; i++) {
        const auto& e = *view[i];
        std::string label = std::to_string(i - start + 1) + ".";
        // Colour alternate rows dimly for readability
        const char* rowColor = (i % 2 == 0) ? "" : C::DIM;
        std::cout << "  " << rowColor
                  << std::left
                  << std::setw(4)  << label
                  << std::setw(38) << e.name
                  << std::setw(14) << e.muscleGroup
                  << std::setw(12) << e.equipment
                  << std::setw(10) << e.type
                  << C::RESET << "\n";
    }

    hr();
    int pages = std::max(1, (total + PAGE - 1) / PAGE);
    std::cout << "  " << C::DIM
              << "Page " << (page+1) << "/" << pages
              << "  |  showing " << (end - start) << " of " << total
              << C::RESET << "\n";
}

// Returns pointer to chosen entry, or nullptr if cancelled
const ExerciseCatalog::Entry* run(const ExerciseCatalog& catalog) {
    const auto& all = catalog.getAllEntries();
    if (all.empty()) {
        err("Exercise catalog is empty. Check exercises.json.");
        return nullptr;
    }

    Filter f;
    int page = 0;

    while (true) {
        header("Exercise Browser");
        printFilterBar(f);

        auto view = applyFilter(all, f);
        int total = (int)view.size();
        int pages = std::max(1, (total + PAGE - 1) / PAGE);
        if (page >= pages) page = pages - 1;

        if (total == 0) {
            warn("No exercises match your filters.");
        } else {
            printPage(view, page, total);
        }

        std::cout << "\n  " << C::BOLD << "Commands:" << C::RESET
                  << C::DIM
                  << "  [1-" << std::min(PAGE, total - page*PAGE) << "] select"
                  << "  [n]ext  [p]rev"
                  << "  [s]earch  [m]uscle  [t]ype  [c]lear  [0] cancel"
                  << C::RESET << "\n";

        std::cout << C::BOLD << "  > " << C::RESET;
        std::string cmd;
        std::getline(std::cin, cmd);
        // trim
        cmd.erase(0, cmd.find_first_not_of(" \t"));
        cmd.erase(cmd.find_last_not_of(" \t") + 1);

        if (cmd == "0") return nullptr;

        // ── Navigation ───────────────────────────────────────────
        if (cmd == "n" || cmd == "N") {
            if (page + 1 < pages) page++; else warn("Already on last page.");
            continue;
        }
        if (cmd == "p" || cmd == "P") {
            if (page > 0) page--; else warn("Already on first page.");
            continue;
        }

        // ── Filters ──────────────────────────────────────────────
        if (cmd == "s" || cmd == "S") {
            std::cout << "  Search name (blank to clear): ";
            std::getline(std::cin, f.search);
            f.search.erase(0, f.search.find_first_not_of(" \t"));
            f.search.erase(f.search.find_last_not_of(" \t") + 1);
            page = 0; continue;
        }
        if (cmd == "m" || cmd == "M") {
            auto groups = uniqueMuscles(all);
            subheader("Select Muscle Group");
            for (int i = 0; i < (int)groups.size(); i++)
                std::cout << "    " << C::CYAN << "[" << (i+1) << "]" << C::RESET
                          << " " << groups[i] << "\n";
            std::cout << "    " << C::CYAN << "[0]" << C::RESET << " All\n";
            int idx = inputInt("> ");
            f.muscleGroup = (idx >= 1 && idx <= (int)groups.size()) ? groups[idx-1] : "";
            page = 0; continue;
        }
        if (cmd == "t" || cmd == "T") {
            subheader("Select Type");
            std::cout << "    " << C::CYAN << "[1]" << C::RESET << " Strength\n";
            std::cout << "    " << C::CYAN << "[2]" << C::RESET << " Cardio\n";
            std::cout << "    " << C::CYAN << "[0]" << C::RESET << " All\n";
            int idx = inputInt("> ");
            if      (idx == 1) f.type = "Strength";
            else if (idx == 2) f.type = "Cardio";
            else               f.type = "";
            page = 0; continue;
        }
        if (cmd == "c" || cmd == "C") { f = Filter{}; page = 0; continue; }

        // ── Numeric selection ────────────────────────────────────
        try {
            int sel = std::stoi(cmd);
            int globalIdx = page * PAGE + sel - 1;
            if (sel >= 1 && globalIdx < total) {
                const auto* chosen = view[globalIdx];
                // Detail card
                std::cout << "\n";
                hr();
                std::cout << "  " << C::BOLD << C::CYAN << chosen->name << C::RESET << "\n";
                std::cout << "  Muscle    : " << chosen->muscleGroup << "\n";
                std::cout << "  Type      : " << chosen->type        << "\n";
                std::cout << "  Equipment : " << chosen->equipment   << "\n";
                if (!chosen->description.empty())
                    std::cout << "  About     : " << chosen->description << "\n";
                hr();
                if (confirm("Add this exercise?")) return chosen;
            } else {
                err("Number out of range. Pick 1-" +
                    std::to_string(std::min(PAGE, total - page*PAGE)) + ".");
            }
        } catch (...) {
            err("Unknown command.");
        }
    }
}

} // namespace Browser

// ============================================================
// ADD SETS  (shared between catalog-pick and custom)
// ============================================================
static void addSetsTo(Exercise* ex) {
    subheader("Add Sets  \xe2\x80\x94  " + ex->getName());
    std::cout << "  " << C::DIM
              << "Commands:  [s] add set   [d] done   [u] undo last set"
              << C::RESET << "\n\n";

    int setNum = 1;
    double lastWeight = 0;
    int    lastRest   = 90;

    while (true) {
        // Show running set list
        if (!ex->getSets().empty()) {
            std::cout << "  " << C::DIM;
            for (int i = 0; i < (int)ex->getSets().size(); i++) {
                const auto& s = ex->getSets()[i];
                std::cout << "  Set " << (i+1) << ": "
                          << s.getWeight() << "kg \xc3\x97 " << s.getReps() << " reps";
                if (i+1 < (int)ex->getSets().size()) std::cout << "   ";
            }
            std::cout << C::RESET << "\n";
        }

        std::cout << C::BOLD << "  Set " << setNum << " (s/d/u): " << C::RESET;
        std::string cmd;
        std::getline(std::cin, cmd);
        cmd.erase(0, cmd.find_first_not_of(" \t"));
        cmd.erase(cmd.find_last_not_of(" \t") + 1);

        if (cmd == "d" || cmd == "done" || cmd.empty()) break;

        if (cmd == "u" || cmd == "undo") {
            // Undo not directly possible via Exercise API since getSets() returns const&
            // We warn the user; a proper undo would require Exercise::removeLastSet()
            warn("Undo not yet supported. Delete the workout and start over if needed.");
            continue;
        }

        if (cmd == "s" || cmd == "set" || cmd == "S") {
            try {
                double w  = inputDoubleOr("  Weight (kg)", lastWeight);
                int    r  = inputInt("  Reps        : ");
                int    rs = inputIntOr("  Rest (sec) ", lastRest);
                if (r <= 0)  { err("Reps must be > 0.");        continue; }
                if (w <  0)  { err("Weight can't be negative."); continue; }
                if (rs < 0)  { err("Rest can't be negative.");   continue; }
                ex->addSet(Set(w, r, rs));
                lastWeight = w;
                lastRest   = rs;
                double oneRM = (r == 1) ? w : w * (1.0 + r / 30.0);
                std::cout << "  " << C::BGREEN << "  \xe2\x9c\x94  Set " << setNum
                          << " logged"
                          << C::DIM << "  (est. 1RM: "
                          << std::fixed << std::setprecision(1) << oneRM << " kg)"
                          << C::RESET << "\n\n";
                setNum++;
            } catch (const std::exception& e) {
                err(std::string("Error: ") + e.what());
            }
            continue;
        }
        err("Unknown command. Type s, d, or u.");
    }

    if (ex->getSets().empty())
        warn("No sets recorded for " + ex->getName() + ".");
    else {
        std::cout << "  " << C::DIM << "  Total volume: "
                  << std::fixed << std::setprecision(1)
                  << ex->getTotalVolume() << " kg"
                  << C::RESET << "\n";
    }
}

// ============================================================
// ADD EXERCISE  (full flow: catalog or custom, then sets)
// ============================================================
void menuAddExercise(Workout* workout, ExerciseCatalog& catalog) {
    while (true) {
        subheader("Add Exercise");
        std::cout << "    " << C::CYAN << "[1]" << C::RESET << " Browse catalog\n";
        std::cout << "    " << C::CYAN << "[2]" << C::RESET << " Enter custom exercise\n";
        std::cout << "    " << C::CYAN << "[0]" << C::RESET << " Done — finish workout\n\n";

        int ch = inputInt("> ");
        if (ch == 0) break;

        Exercise* ex = nullptr;

        // ── Catalog browser ──────────────────────────────────────
        if (ch == 1) {
            const ExerciseCatalog::Entry* chosen = Browser::run(catalog);
            if (!chosen) continue;

            if (chosen->type == "Cardio") {
                double dist = inputDouble("  Distance (km, 0 to skip): ");
                double pace = inputDouble("  Pace (min/km, 0 to skip): ");
                ex = new CardioExercise(chosen->name, chosen->muscleGroup,
                                        dist, pace, chosen->description);
            } else {
                std::string eq = chosen->equipment.empty() ? "barbell" : chosen->equipment;
                ex = new StrengthExercise(chosen->name, chosen->muscleGroup,
                                          eq, chosen->description);
            }

        // ── Custom exercise ──────────────────────────────────────
        } else if (ch == 2) {
            subheader("Custom Exercise");
            std::string name = inputLine("Name: ");
            if (name.empty()) { err("Name cannot be empty."); continue; }

            // Type selection with numbered menu instead of free-text
            std::cout << "    " << C::CYAN << "[1]" << C::RESET << " Strength\n";
            std::cout << "    " << C::CYAN << "[2]" << C::RESET << " Cardio\n";
            int typeChoice = inputInt("Type > ");
            std::string type = (typeChoice == 2) ? "Cardio" : "Strength";

            std::string mg   = inputLine("Muscle group   : ");
            std::string eq   = inputLine("Equipment      : ");
            std::string desc = inputLine("Description    : ");
            if (eq.empty()) eq = "barbell";

            if (type == "Cardio") {
                double dist = inputDouble("  Distance (km, 0 to skip): ");
                double pace = inputDouble("  Pace (min/km, 0 to skip): ");
                ex = new CardioExercise(name, mg, dist, pace, desc);
            } else {
                ex = new StrengthExercise(name, mg, eq, desc);
            }

        } else {
            err("Invalid choice.");
            continue;
        }

        addSetsTo(ex);
        workout->addExercise(ex);

        // Mini summary card
        std::cout << "\n  " << C::BGREEN << "\xe2\x9c\x94  " << ex->getName() << " added"
                  << C::DIM << "  ("
                  << ex->getSets().size() << " sets, "
                  << std::fixed << std::setprecision(1) << ex->getTotalVolume() << " kg total)"
                  << C::RESET << "\n";

        if (!confirm("Add another exercise?")) break;
    }
}

// ============================================================
// NEW WORKOUT
// ============================================================
void menuNewWorkout(User& user) {
    header("New Workout");

    // Date — default to today
    std::string today = todayDate();
    std::cout << C::BOLD << "  Date (YYYY-MM-DD)" << C::DIM << " [" << today << "]"
              << C::RESET << ": ";
    std::string date;
    std::getline(std::cin, date);
    date.erase(0, date.find_first_not_of(" \t"));
    date.erase(date.find_last_not_of(" \t") + 1);
    if (date.empty()) date = today;

    int dur = inputInt("Duration (min)    : ");

    // Mood with numbered choices
    subheader("How are you feeling?");
    std::cout << "    " << C::CYAN << "[1]" << C::RESET << " Great \xf0\x9f\x94\xa5\n";
    std::cout << "    " << C::CYAN << "[2]" << C::RESET << " Good  \xf0\x9f\x91\x8d\n";
    std::cout << "    " << C::CYAN << "[3]" << C::RESET << " Average \xf0\x9f\x98\x90\n";
    std::cout << "    " << C::CYAN << "[4]" << C::RESET << " Bad   \xf0\x9f\x98\xb4\n";
    int moodChoice = inputInt("> ");
    std::string moodMap[] = {"Great", "Good", "Average", "Bad"};
    std::string mood = (moodChoice >= 1 && moodChoice <= 4) ? moodMap[moodChoice-1] : "Good";

    std::string notes = inputLine("Notes (optional) : ");

    Workout* w = new Workout(date, dur, notes, mood);
    menuAddExercise(w, user.getCatalog());

    if (w->getExercises().empty() && !confirm("Workout has no exercises. Save anyway?")) {
        delete w;
        warn("Workout discarded.");
        return;
    }

    user.addWorkout(w);

    // Summary
    std::cout << "\n";
    hr('=');
    std::cout << "  " << C::BGREEN << "\xe2\x9c\x94  Workout saved!" << C::RESET << "\n";
    std::cout << "  " << C::DIM
              << date << "  |  " << dur << " min  |  Mood: " << mood
              << "  |  " << w->getExercises().size() << " exercises"
              << "  |  " << std::fixed << std::setprecision(1) << w->getTotalVolume() << " kg"
              << C::RESET << "\n";
    hr('=');
}

// ============================================================
// VIEW WORKOUT HISTORY
// ============================================================
void menuWorkoutHistory(User& user) {
    header("Workout History");

    const auto& workouts = user.getWorkouts();
    if (workouts.empty()) {
        info("No workouts logged yet. Go lift something!");
        return;
    }

    // List them newest-first
    for (int i = (int)workouts.size() - 1; i >= 0; i--) {
        const Workout* w = workouts[i];
        std::cout << "  " << C::CYAN << "[" << i << "]" << C::RESET
                  << " " << C::BOLD << w->getDate() << C::RESET
                  << C::DIM << "  " << w->getDuration() << " min"
                  << "  |  Mood: " << w->getMood()
                  << "  |  " << w->getExercises().size() << " exercises"
                  << "  |  " << std::fixed << std::setprecision(0)
                  << w->getTotalVolume() << " kg"
                  << C::RESET << "\n";
    }

    hr();
    std::cout << "  " << C::DIM << "Enter index to view details, or 0 to go back." << C::RESET << "\n";
    int idx = inputInt("> ");
    if (idx == 0) return;
    if (idx < 0 || idx >= (int)workouts.size()) { err("Invalid index."); return; }

    // Detail view
    std::cout << "\n";
    workouts[idx]->print(false);

    if (confirm("Delete this workout?")) {
        if (user.deleteWorkout(idx))
            ok("Workout deleted.");
        else
            err("Could not delete workout.");
    }
}

// ============================================================
// EDIT PRs
// ============================================================
void menuEditPRs(User& user) {
    header("Personal Records");
    auto& prs = user.getPRs();

    if (prs.empty()) {
        info("No PRs yet. Log some workouts first!");
        return;
    }

    ProgressTracker::printAllPRs(prs);

    std::string exName = inputLine("Exercise name to edit (blank to cancel): ");
    if (exName.empty()) return;

    auto it = prs.find(exName);
    if (it == prs.end()) {
        if (!confirm("No PR found for \"" + exName + "\". Create a new entry?")) return;
    }

    auto& pr = prs[exName];
    subheader("Editing PR for: " + exName);
    info("Press Enter to keep the current value.");
    std::cout << "\n";

    auto editDouble = [](const std::string& label, double cur) -> double {
        std::cout << C::BOLD << "  " << label
                  << C::DIM << " [current: " << std::fixed << std::setprecision(2) << cur << "]"
                  << C::RESET << ": ";
        std::string s; std::getline(std::cin, s);
        s.erase(0, s.find_first_not_of(" \t"));
        s.erase(s.find_last_not_of(" \t") + 1);
        if (s.empty()) return cur;
        try { return std::stod(s); } catch (...) { return cur; }
    };
    auto editInt = [](const std::string& label, int cur) -> int {
        std::cout << C::BOLD << "  " << label
                  << C::DIM << " [current: " << cur << "]"
                  << C::RESET << ": ";
        std::string s; std::getline(std::cin, s);
        s.erase(0, s.find_first_not_of(" \t"));
        s.erase(s.find_last_not_of(" \t") + 1);
        if (s.empty()) return cur;
        try { return std::stoi(s); } catch (...) { return cur; }
    };

    pr.bestWeight = editDouble("Best weight (kg)", pr.bestWeight);
    pr.bestReps   = editInt   ("Best reps        ", pr.bestReps);
    pr.bestVolume = editDouble("Best volume (kg) ", pr.bestVolume);
    pr.best1RM    = editDouble("Best 1RM (kg)    ", pr.best1RM);

    std::cout << C::BOLD << "  Date (YYYY-MM-DD)"
              << C::DIM << " [current: " << pr.date << "]"
              << C::RESET << ": ";
    std::string ds; std::getline(std::cin, ds);
    ds.erase(0, ds.find_first_not_of(" \t"));
    ds.erase(ds.find_last_not_of(" \t") + 1);
    if (!ds.empty()) pr.date = ds;

    ok("PR updated for " + exName + ".");
}

// ============================================================
// 1RM CALCULATOR
// ============================================================
void menuCalc1RM() {
    header("1RM Calculator");
    info("Uses the Epley formula: 1RM = weight \xc3\x97 (1 + reps/30)");
    std::cout << "\n";

    while (true) {
        double w = inputDouble("Weight used (kg): ");
        int    r = inputInt("Reps performed  : ");

        if (r <= 0) { err("Reps must be at least 1."); continue; }
        if (w <= 0) { err("Weight must be positive."); continue; }

        double oneRM = (r == 1) ? w : w * (1.0 + r / 30.0);

        std::cout << "\n";
        hr();
        std::cout << "  " << C::BGREEN << "Estimated 1RM: "
                  << std::fixed << std::setprecision(1) << oneRM << " kg"
                  << C::RESET << "\n";
        hr();

        // Training percentage table
        std::cout << "\n  " << C::BOLD << std::left
                  << std::setw(8)  << "%"
                  << std::setw(12) << "Weight"
                  << std::setw(16) << "Typical Use"
                  << C::RESET << "\n";
        hr();

        struct Row { int pct; const char* use; };
        Row rows[] = {
            {100, "1RM attempt"},
            { 95, "Heavy singles"},
            { 90, "Strength (1-3 reps)"},
            { 85, "Strength (3-5 reps)"},
            { 80, "Hypertrophy (6-8 reps)"},
            { 75, "Hypertrophy (8-10 reps)"},
            { 70, "Volume (10-12 reps)"},
            { 65, "Volume (12-15 reps)"},
            { 60, "Endurance (15+ reps)"},
        };
        for (const auto& row : rows) {
            double kg = oneRM * row.pct / 100.0;
            const char* col = (row.pct >= 90) ? C::RED :
                              (row.pct >= 75) ? C::YELLOW : C::GREEN;
            std::cout << "  " << col << std::left
                      << std::setw(8)  << (std::to_string(row.pct) + "%")
                      << std::fixed << std::setprecision(1)
                      << std::setw(12) << (std::to_string((int)(kg*10)/10.0).substr(0,5) + " kg")
                      << C::RESET
                      << C::DIM << row.use << C::RESET << "\n";
        }
        hr();

        if (!confirm("Calculate another?")) break;
        std::cout << "\n";
    }
}

// ============================================================
// TEMPLATES
// ============================================================
void menuTemplates(User& user) {
    auto& templates = user.getTemplates();

    while (true) {
        header("Workout Templates");
        std::cout << "    " << C::CYAN << "[1]" << C::RESET << " View templates\n";
        std::cout << "    " << C::CYAN << "[2]" << C::RESET << " Create template\n";
        std::cout << "    " << C::CYAN << "[3]" << C::RESET << " Use template for new workout\n";
        std::cout << "    " << C::CYAN << "[4]" << C::RESET << " Delete template\n";
        std::cout << "    " << C::CYAN << "[0]" << C::RESET << " Back\n\n";
        int ch = inputInt("> ");
        if (ch == 0) break;

        // ── View ─────────────────────────────────────────────────
        if (ch == 1) {
            if (templates.empty()) { info("No templates saved yet."); continue; }
            subheader("Saved Templates");
            for (int i = 0; i < (int)templates.size(); i++) {
                const auto& t = templates[i];
                std::cout << "  " << C::BOLD << C::CYAN << "[" << i << "] " << t.name << C::RESET;
                if (!t.notes.empty())
                    std::cout << C::DIM << "  — " << t.notes << C::RESET;
                std::cout << "\n";
                for (const auto& ex : t.exercises)
                    std::cout << "      " << C::DIM << "\xe2\x80\xa2 "
                              << ex.exName << "  [" << ex.muscleGroup << "]"
                              << C::RESET << "\n";
                std::cout << "\n";
            }

        // ── Create ───────────────────────────────────────────────
        } else if (ch == 2) {
            subheader("Create Template");
            WorkoutTemplate t;
            t.name = inputLine("Template name  : ");
            if (t.name.empty()) { err("Name cannot be empty."); continue; }
            t.notes = inputLine("Notes (optional): ");

            info("Add exercises. Leave name blank when done.");
            std::cout << "\n";
            while (true) {
                std::string n = inputLine("  Exercise name (blank to stop): ");
                if (n.empty()) break;
                std::string mg   = inputLine("  Muscle group                : ");
                std::string type = inputLine("  Type (Strength/Cardio)      : ");
                if (type.empty()) type = "Strength";
                t.exercises.push_back({n, mg, type});
                ok("Added: " + n);
            }

            if (t.exercises.empty())
                warn("Template has no exercises.");

            templates.push_back(t);
            ok("Template \"" + t.name + "\" saved with " +
               std::to_string(t.exercises.size()) + " exercises.");

        // ── Use template ─────────────────────────────────────────
        } else if (ch == 3) {
            if (templates.empty()) { info("No templates yet."); continue; }
            subheader("Use Template");
            for (int i = 0; i < (int)templates.size(); i++)
                std::cout << "    " << C::CYAN << "[" << i << "]" << C::RESET
                          << " " << templates[i].name << "\n";
            int idx = inputInt("> ");
            if (idx < 0 || idx >= (int)templates.size()) { err("Invalid index."); continue; }

            const auto& tmpl = templates[idx];
            info("Starting workout from template: " + tmpl.name);

            std::string today = todayDate();
            std::cout << C::BOLD << "  Date (YYYY-MM-DD)" << C::DIM << " [" << today << "]"
                      << C::RESET << ": ";
            std::string date; std::getline(std::cin, date);
            date.erase(0, date.find_first_not_of(" \t"));
            date.erase(date.find_last_not_of(" \t") + 1);
            if (date.empty()) date = today;

            int dur = inputInt("Duration (min): ");
            Workout* w = new Workout(date, dur, "From template: " + tmpl.name, "Good");

            for (const auto& entry : tmpl.exercises) {
                Exercise* ex = new StrengthExercise(entry.exName, entry.muscleGroup);
                addSetsTo(ex);
                w->addExercise(ex);
            }

            // allow adding extra exercises
            if (confirm("Add extra exercises?"))
                menuAddExercise(w, user.getCatalog());

            user.addWorkout(w);
            ok("Workout saved from template \"" + tmpl.name + "\".");

        // ── Delete ───────────────────────────────────────────────
        } else if (ch == 4) {
            if (templates.empty()) { info("No templates to delete."); continue; }
            subheader("Delete Template");
            for (int i = 0; i < (int)templates.size(); i++)
                std::cout << "    " << C::CYAN << "[" << i << "]" << C::RESET
                          << " " << templates[i].name << "\n";
            int idx = inputInt("> ");
            if (idx < 0 || idx >= (int)templates.size()) { err("Invalid index."); continue; }
            if (confirm("Delete \"" + templates[idx].name + "\"?")) {
                templates.erase(templates.begin() + idx);
                ok("Template deleted.");
            }
        }
    }
}

// ============================================================
// BODY WEIGHT TRACKER
// ============================================================
void menuBodyWeight(User& user) {
    while (true) {
        header("Body Weight Tracker");
        std::cout << "    " << C::CYAN << "[1]" << C::RESET << " Log weight\n";
        std::cout << "    " << C::CYAN << "[2]" << C::RESET << " View chart\n";
        std::cout << "    " << C::CYAN << "[0]" << C::RESET << " Back\n\n";
        int ch = inputInt("> ");
        if (ch == 0) break;

        if (ch == 1) {
            std::string today = todayDate();
            std::cout << C::BOLD << "  Date (YYYY-MM-DD)" << C::DIM << " [" << today << "]"
                      << C::RESET << ": ";
            std::string date; std::getline(std::cin, date);
            date.erase(0, date.find_first_not_of(" \t"));
            date.erase(date.find_last_not_of(" \t") + 1);
            if (date.empty()) date = today;

            double bw = inputDouble("  Weight (kg): ");
            if (bw <= 0) { err("Weight must be positive."); continue; }
            user.logBodyWeight(date, bw);
            ok("Logged " + std::to_string(bw).substr(0,5) + " kg on " + date + ".");

        } else if (ch == 2) {
            ProgressTracker::bodyWeightChart(user.getBWLog());
        }
    }
}

// ============================================================
// MUSCLE FREQUENCY
// ============================================================
void menuMuscleFrequency(const User& user) {
    header("Muscle Group Frequency");

    std::map<std::string, int> freq;
    for (const auto* w : user.getWorkouts())
        for (const auto* ex : w->getExercises())
            freq[ex->getMuscleGroup()]++;

    if (freq.empty()) { info("No workout data yet."); return; }

    // Sort by frequency descending
    std::vector<std::pair<std::string, int>> sorted(freq.begin(), freq.end());
    std::sort(sorted.begin(), sorted.end(),
        [](const auto& a, const auto& b){ return a.second > b.second; });

    int maxFreq = sorted.front().second;
    const int BAR = 28;

    std::cout << "  " << C::BOLD << std::left
              << std::setw(16) << "Muscle Group"
              << "  " << std::setw(BAR) << "Frequency"
              << "  Sets\n" << C::RESET;
    hr();

    for (const auto& kv : sorted) {
        int bars = (maxFreq > 0) ? (int)((double)kv.second / maxFreq * BAR) : 0;
        // pick colour by frequency tier
        const char* col = (bars >= BAR * 0.66) ? C::GREEN :
                          (bars >= BAR * 0.33) ? C::YELLOW : C::RED;
        std::cout << "  " << std::left << std::setw(16) << kv.first << "  ";
        std::cout << col;
        for (int i = 0; i < bars; i++)       std::cout << "\xe2\x96\x88";
        std::cout << C::DIM;
        for (int i = bars; i < BAR; i++)     std::cout << "\xe2\x96\x91";
        std::cout << C::RESET;
        std::cout << "  " << C::BOLD << kv.second << C::RESET << "x\n";
    }
    hr();

    // Imbalance hint
    if (sorted.size() >= 2) {
        const auto& top = sorted.front();
        const auto& bot = sorted.back();
        if (top.second > bot.second * 3)
            warn("Possible imbalance: " + top.first + " trained 3x more than " + bot.first + ".");
    }
    std::cout << "\n";
}