#include "User.h"
#include <iostream>
#include <fstream>
#include <sstream>

User::User(const std::string& n, int a, double w)
    : name(n), age(a), weight(w) {}

User::~User() {
    for (auto* w : workouts) delete w;
}

const std::string& User::getName() const { return name; }
int User::getAge() const { return age; }
double User::getWeight() const { return weight; }
void User::setWeight(double w) { weight = w; }

ExerciseCatalog& User::getCatalog() { return catalog; }
std::map<std::string, PersonalRecord>& User::getPRs() { return prs; }
std::map<std::string, double>& User::getBWLog() { return bodyWeightLog; }
std::vector<WorkoutTemplate>& User::getTemplates() { return templates; }
const std::vector<Workout*>& User::getWorkouts() const { return workouts; }

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
              << " | Weight: " << weight << " kg ===\n";
    std::cout << "  Total workouts: " << workouts.size() << "\n\n";
}

void User::saveToFile() {
    std::ofstream out("workout_data.txt");
    if (!out) return;

    out << name << "\n" << age << "\n" << weight << "\n";

    out << bodyWeightLog.size() << "\n";
    for (const auto& pair : bodyWeightLog)
        out << pair.first << " " << pair.second << "\n";

    out << workouts.size() << "\n";
    for (auto* w : workouts) {
        out << w->getDate() << "|" << w->getDuration() << "|" << w->getMood() << "|" << w->getNotes() << "\n";
        out << w->getExercises().size() << "\n";
        for (auto* ex : w->getExercises()) {
            out << ex->getType() << "|" << ex->getName() << "|"
                << ex->getMuscleGroup() << "|" << ex->getDescription() << "|";
            if (ex->getType() == "Strength")
                out << static_cast<StrengthExercise*>(ex)->getEquipment() << "\n";
            else
                out << static_cast<CardioExercise*>(ex)->getDistance() << "|"
                    << static_cast<CardioExercise*>(ex)->getPace() << "\n";

            out << ex->getSets().size() << "\n";
            for (const auto& s : ex->getSets())
                out << s.getWeight() << " " << s.getReps() << " " << s.getRest() << "\n";
        }
    }
    out.close();
}

User* User::loadFromFile() {
    std::ifstream in("workout_data.txt");
    if (!in) return nullptr;

    std::string n, line;
    int a; double w;
    std::getline(in, n);
    in >> a >> w;

    User* user = new User(n, a, w);

    int bwCount; in >> bwCount;
    for (int i = 0; i < bwCount; i++) {
        std::string d; double bwVal;
        in >> d >> bwVal;
        user->bodyWeightLog[d] = bwVal;
    }

    int wCount; in >> wCount;
    std::getline(in, line); // clear buffer
    for (int i = 0; i < wCount; i++) {
        std::getline(in, line);
        std::stringstream ss(line);
        std::string d, dur, mood, notes;
        std::getline(ss, d, '|'); std::getline(ss, dur, '|');
        std::getline(ss, mood, '|'); std::getline(ss, notes, '|');

        Workout* workout = new Workout(d, std::stoi(dur), notes, mood);
        int exCount; in >> exCount;
        std::getline(in, line);

        for (int j = 0; j < exCount; j++) {
            std::getline(in, line);
            std::stringstream ssEx(line);
            std::string type, ename, mg, desc;
            std::getline(ssEx, type, '|'); std::getline(ssEx, ename, '|');
            std::getline(ssEx, mg, '|'); std::getline(ssEx, desc, '|');

            Exercise* ex = nullptr;
            if (type == "Strength") {
                std::string eq; std::getline(ssEx, eq);
                ex = new StrengthExercise(ename, mg, eq, desc);
            } else {
                std::string dist, pace;
                std::getline(ssEx, dist, '|'); std::getline(ssEx, pace);
                ex = new CardioExercise(ename, mg, std::stod(dist), std::stod(pace), desc);
            }

            int sCount; in >> sCount;
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
    return user;
}
