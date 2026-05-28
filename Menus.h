#pragma once
#include "User.h"

// ── Input helpers (used across all menus and main.cpp) ──────
std::string inputLine(const std::string& prompt);
int         inputInt(const std::string& prompt);
double      inputDouble(const std::string& prompt);

// ── Menu functions ───────────────────────────────────────────
void menuAddExercise(Workout* workout, ExerciseCatalog& catalog);
void menuNewWorkout(User& user);
void menuWorkoutHistory(User& user);       // replaces bare listWorkouts call in main
void menuEditPRs(User& user);
void menuCalc1RM();
void menuTemplates(User& user);
void menuBodyWeight(User& user);
void menuMuscleFrequency(const User& user);