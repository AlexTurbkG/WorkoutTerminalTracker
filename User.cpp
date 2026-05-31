#include "User.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <windows.h>   // for FindFirstFile / FindNextFile

// ============================================================
// CONSTRUCTOR / DESTRUCTOR
// ============================================================
User::User(const std::string& n, int a, double w)
    : name(n), age(a), weight(w) {}

User::~User() {
    for (auto* w : workouts) delete w;
}

// ============================================================
// GETTERS / SETTERS
// ============================================================
const std::string& User::getName()   const { return name; }
int                User::getAge()    const { return age; }
double             User::getWeight() const { return weight; }
void               User::setWeight(double w) { weight = w; }

ExerciseCatalog&                        User::getCatalog()   { return catalog; }
std::map<std::string, PersonalRecord>&  User::getPRs()       { return prs; }
std::map<std::string, double>&          User::getBWLog()     { return bodyWeightLog; }
std::vector<WorkoutTemplate>&           User::getTemplates() { return templates; }
const std::vector<Workout*>&            User::getWorkouts()  const { return workouts; }

// ============================================================
// WORKOUT MANAGEMENT
// ============================================================
void User::addWorkout(Workout* w) {
    workouts.push_back(w);
    ProgressTracker::updatePRs(*w, prs);
}

bool User::deleteWorkout(int idx) {
    if (idx < 0 || idx >= (int)workouts.size()) return false;
    delete workouts[idx];
    workouts.erase(workouts.begin() + idx);
    return true;
}

Workout* User::getWorkout(int idx) {
    if (idx < 0 || idx >= (int)workouts.size()) return nullptr;
    return workouts[idx];
}

int User::workoutCount() const { return (int)workouts.size(); }

void User::listWorkouts(bool brief) const {
    if (workouts.empty()) { std::cout << "  No workouts logged.\n"; return; }
    for (int i = 0; i < (int)workouts.size(); i++) {
        std::cout << "  [" << i << "] ";
        workouts[i]->print(brief);
    }
}

void User::logBodyWeight(const std::string& date, double bw) {
    bodyWeightLog[date] = bw;
}

void User::printProfile() const {
    std::cout << "\n=== Profile: " << name << " | Age: " << age
              << " | Weight: " << weight << " kg ===\n"
              << "  Total workouts: " << workouts.size() << "\n\n";
}

// ============================================================
// FILENAME HELPER
// Converts a username to a safe filename.
// "John Doe" -> "user_John_Doe.txt"
// Replaces spaces with underscores, strips anything non-alphanumeric/underscore.
// ============================================================
std::string User::usernameToFilename(const std::string& n) {
    std::string safe;
    for (char c : n) {
        if (std::isalnum((unsigned char)c)) safe += c;
        else if (c == ' ')                  safe += '_';
        // skip all other chars
    }
    if (safe.empty()) safe = "user";
    return "user_" + safe + ".txt";
}

// ============================================================
// SAVE TO FILE
// ============================================================
void User::saveToFile() const {
    std::string filename = usernameToFilename(name);
    std::ofstream out(filename);
    if (!out) {
        std::cerr << "  Error: could not write to " << filename << "\n";
        return;
    }

    // ── Header ───────────────────────────────────────────────
    out << name << "\n" << age << "\n" << weight << "\n";

    // ── Body weight log ──────────────────────────────────────
    out << bodyWeightLog.size() << "\n";
    for (const auto& kv : bodyWeightLog)
        out << kv.first << " " << kv.second << "\n";

    // ── Workouts ─────────────────────────────────────────────
    out << workouts.size() << "\n";
    for (const auto* w : workouts) {
        out << w->getDate()     << "|"
            << w->getDuration() << "|"
            << w->getMood()     << "|"
            << w->getNotes()    << "\n";
        out << w->getExercises().size() << "\n";
        for (const auto* ex : w->getExercises()) {
            out << ex->getType()        << "|"
                << ex->getName()        << "|"
                << ex->getMuscleGroup() << "|"
                << ex->getDescription() << "|";
            if (ex->getType() == "Strength")
                out << static_cast<const StrengthExercise*>(ex)->getEquipment() << "\n";
            else
                out << static_cast<const CardioExercise*>(ex)->getDistance() << "|"
                    << static_cast<const CardioExercise*>(ex)->getPace()     << "\n";

            out << ex->getSets().size() << "\n";
            for (const auto& s : ex->getSets())
                out << s.getWeight() << " " << s.getReps() << " " << s.getRest() << "\n";
        }
    }

    // ── Templates ────────────────────────────────────────────
    out << templates.size() << "\n";
    for (const auto& t : templates) {
        // Escape pipe chars in name/notes just in case
        out << t.name << "|" << t.notes << "\n";
        out << t.exercises.size() << "\n";
        for (const auto& e : t.exercises)
            out << e.exName << "|" << e.muscleGroup << "|" << e.type << "\n";
    }

    out.close();
}

// ============================================================
// LOAD FROM FILE
// ============================================================
User* User::loadFromFile(const std::string& filename) {
    std::ifstream in(filename);
    if (!in) return nullptr;

    std::string line;

    // ── Header ───────────────────────────────────────────────
    std::string n;
    int    a;
    double w;
    std::getline(in, n);
    in >> a >> w;
    std::getline(in, line); // consume rest of weight line

    User* user = new User(n, a, w);

    // ── Body weight log ──────────────────────────────────────
    int bwCount = 0;
    in >> bwCount;
    std::getline(in, line);
    for (int i = 0; i < bwCount; i++) {
        std::string d; double bwVal;
        in >> d >> bwVal;
        user->bodyWeightLog[d] = bwVal;
    }
    if (bwCount > 0) std::getline(in, line); // consume trailing newline

    // ── Workouts ─────────────────────────────────────────────
    int wCount = 0;
    in >> wCount;
    std::getline(in, line);
    for (int i = 0; i < wCount; i++) {
        std::getline(in, line);
        std::stringstream ss(line);
        std::string d, dur, mood, notes;
        std::getline(ss, d,    '|');
        std::getline(ss, dur,  '|');
        std::getline(ss, mood, '|');
        std::getline(ss, notes,'|');

        Workout* workout = new Workout(d, std::stoi(dur), notes, mood);

        int exCount = 0;
        in >> exCount;
        std::getline(in, line);
        for (int j = 0; j < exCount; j++) {
            std::getline(in, line);
            std::stringstream ssEx(line);
            std::string type, ename, mg, desc;
            std::getline(ssEx, type,  '|');
            std::getline(ssEx, ename, '|');
            std::getline(ssEx, mg,    '|');
            std::getline(ssEx, desc,  '|');

            Exercise* ex = nullptr;
            if (type == "Strength") {
                std::string eq; std::getline(ssEx, eq);
                ex = new StrengthExercise(ename, mg, eq, desc);
            } else {
                std::string dist, pace;
                std::getline(ssEx, dist, '|');
                std::getline(ssEx, pace);
                ex = new CardioExercise(ename, mg, std::stod(dist), std::stod(pace), desc);
            }

            int sCount = 0;
            in >> sCount;
            for (int k = 0; k < sCount; k++) {
                double wt; int rp, rs;
                in >> wt >> rp >> rs;
                ex->addSet(Set(wt, rp, rs));
            }
            std::getline(in, line);
            workout->addExercise(ex);
        }
        user->addWorkout(workout);
    }

    // ── Templates ────────────────────────────────────────────
    // Older save files won't have this section — guard with eof check
    int tCount = 0;
    if (in >> tCount) {
        std::getline(in, line); // consume newline after count
        for (int i = 0; i < tCount; i++) {
            std::getline(in, line);
            std::stringstream ss(line);
            WorkoutTemplate t;
            std::getline(ss, t.name,  '|');
            std::getline(ss, t.notes, '|');

            int eCount = 0;
            in >> eCount;
            std::getline(in, line);
            for (int j = 0; j < eCount; j++) {
                std::getline(in, line);
                std::stringstream ssE(line);
                WorkoutTemplate::ExEntry e;
                std::getline(ssE, e.exName,      '|');
                std::getline(ssE, e.muscleGroup, '|');
                std::getline(ssE, e.type,        '|');
                t.exercises.push_back(e);
            }
            user->templates.push_back(t);
        }
    }

    return user;
}

// ============================================================
// LIST SAVED USERS
// Scans the current directory for files matching "user_*.txt"
// and returns the list of filenames.
// ============================================================
std::vector<std::string> User::listSavedUsers() {
    std::vector<std::string> files;
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA("user_*.txt", &fd);
    if (hFind == INVALID_HANDLE_VALUE) return files;
    do {
        files.push_back(fd.cFileName);
    } while (FindNextFileA(hFind, &fd));
    FindClose(hFind);
    std::sort(files.begin(), files.end());
    return files;
}