#pragma once
#include "User.h"

// Input helpers
std::string inputLine(const std::string& prompt);
int inputInt(const std::string& prompt);
double inputDouble(const std::string& prompt);

// Menu functions
void menuAddExercise(Workout* workout, ExerciseCatalog& catalog);
void menuNewWorkout(User& user);
void menuEditPRs(User& user);
void menuCalc1RM();
void menuTemplates(User& user);
void menuBodyWeight(User& user);
void menuMuscleFrequency(const User& user);
