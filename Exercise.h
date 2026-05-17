#pragma once
#include "Set.h"
#include <string>
#include <vector>

// ============================================================
// EXERCISE (Abstract)
// ============================================================
class Exercise {
protected:
    std::string name;
    std::string muscleGroup;
    std::string description;
    std::vector<Set> sets;

public:
    Exercise(const std::string& n, const std::string& mg, const std::string& desc = "");
    virtual ~Exercise() = default;

    virtual std::string getType() const = 0;
    virtual void printDetails() const = 0;

    const std::string& getName() const;
    const std::string& getMuscleGroup() const;
    const std::string& getDescription() const;
    const std::vector<Set>& getSets() const;

    void setName(const std::string& n);
    void setDescription(const std::string& d);
    void addSet(const Set& s);

    double getTotalVolume() const;
    double getBest1RM() const;
    double getMaxWeight() const;
    int getMaxReps() const;

    void printSets() const;
};

// ============================================================
// STRENGTH EXERCISE
// ============================================================
class StrengthExercise : public Exercise {
private:
    std::string equipment;

public:
    StrengthExercise(const std::string& n, const std::string& mg,
                     const std::string& eq = "barbell", const std::string& desc = "");

    std::string getType() const override;
    const std::string& getEquipment() const;
    void printDetails() const override;
};

// ============================================================
// CARDIO EXERCISE
// ============================================================
class CardioExercise : public Exercise {
private:
    double distanceKm;
    double paceMinPerKm;

public:
    CardioExercise(const std::string& n, const std::string& mg,
                   double dist = 0, double pace = 0, const std::string& desc = "");

    std::string getType() const override;
    double getDistance() const;
    double getPace() const;
    void setDistance(double d);
    void setPace(double p);
    void printDetails() const override;
};
