#include "Set.h"

Set::Set(double w, int r, int rest) {
    setWeight(w);
    setReps(r);
    setRest(rest);
}

void Set::setWeight(double w) {
    if (w < 0) throw std::invalid_argument("Weight cannot be negative.");
    weight = w;
}
void Set::setReps(int r) {
    if (r <= 0) throw std::invalid_argument("Reps must be positive.");
    reps = r;
}
void Set::setRest(int r) {
    if (r < 0) throw std::invalid_argument("Rest time cannot be negative.");
    restSeconds = r;
}

double Set::getWeight() const { return weight; }
int Set::getReps() const { return reps; }
int Set::getRest() const { return restSeconds; }
double Set::getVolume() const { return weight * reps; }

double Set::calc1RM() const {
    if (reps == 1) return weight;
    return weight * (1.0 + reps / 30.0);
}

void Set::print(int idx) const {
    std::cout << "    Set " << idx << ": " << weight << " kg x " << reps
              << " reps  [rest: " << restSeconds << "s]  (1RM est: "
              << std::fixed << std::setprecision(1) << calc1RM() << " kg)\n";
}
