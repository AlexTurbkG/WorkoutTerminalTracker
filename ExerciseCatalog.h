#pragma once
#include <string>
#include <vector>

class ExerciseCatalog {
private:
    struct CatalogEntry {
        std::string name;
        std::string muscleGroup;
        std::string type;
        std::string equipment;
        std::string description;
    };
    std::vector<CatalogEntry> entries;

public:
    ExerciseCatalog();

    void addEntry(const std::string& name, const std::string& mg,
                  const std::string& type, const std::string& eq, const std::string& desc);

    void listAll() const;
    void listByMuscle(const std::string& mg) const;

    bool find(const std::string& name, std::string& outType, std::string& outMG,
              std::string& outEq, std::string& outDesc) const;
};
