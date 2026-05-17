#pragma once
#include <stdexcept>
#include <iostream>
#include <iomanip>

class Set {
private:
    double weight;
    int reps;
    int restSeconds;

public:
    Set(double w, int r, int rest = 60);

    void setWeight(double w);
    void setReps(int r);
    void setRest(int r);

    double getWeight() const;
    int getReps() const;
    int getRest() const;
    double getVolume() const;
    double calc1RM() const;

    void print(int idx) const;
};
