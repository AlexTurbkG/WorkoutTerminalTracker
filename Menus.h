#pragma once
#include "User.h"

// ── Input helpers ────────────────────────────────────────────
std::string inputLine(const std::string& prompt);
int         inputInt(const std::string& prompt);
double      inputDouble(const std::string& prompt);

// ── Menu functions ───────────────────────────────────────────
void menuAddExercise(Workout* workout, ExerciseCatalog& catalog);
void menuNewWorkout(User& user);
void menuWorkoutHistory(User& user);
void menuEditPRs(User& user);
void menuCalc1RM();
void menuTemplates(User& user);
void menuBodyWeight(User& user);
void menuMuscleFrequency(const User& user);

// ── Return codes for the main loop ───────────────────────────
// runUserSession() returns one of these so main() knows what to do next.
enum class SessionResult {
    Exit,       // user chose Save & Exit  -> quit the program
    Logout,     // user chose Log Out      -> go back to user-select screen
};

SessionResult runUserSession(User& user);