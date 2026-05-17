#include "User.h"
#include "Menus.h"
#include "ProgressTracker.h"
#include <windows.h>
#include <iostream>

void printBanner() {
    std::cout << R"(
╔══════════════════════════════════════════════╗
║          WORKOUT TRACKER  v1.2               ║
║   Log. Track. Progress. Dominate.            ║
╚══════════════════════════════════════════════╝
)";
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    User* user = User::loadFromFile();

    if (!user) {
        printBanner();
        std::cout << "-- Create Profile --\n";
        std::string uname  = inputLine("  Name: ");
        int uage           = inputInt("  Age: ");
        double uweight     = inputDouble("  Weight (kg): ");
        user = new User(uname, uage, uweight);
    } else {
        printBanner();
        std::cout << "Welcome back, " << user->getName() << "!\n";
    }

    while (true) {
        std::cout << "\n========== MAIN MENU ==========\n";
        std::cout << "1. Log New Workout\n";
        std::cout << "2. View Workout History\n";
        std::cout << "3. Personal Records (PRs)\n";
        std::cout << "4. Volume Progress Chart\n";
        std::cout << "5. Weekly Summary\n";
        std::cout << "6. Body Weight Tracker\n";
        std::cout << "7. Muscle Frequency\n";
        std::cout << "8. 1RM Calculator\n";
        std::cout << "9. Workout Templates\n";
        std::cout << "0. Exit\n";
        std::cout << "===============================\n";

        int choice = inputInt("Select option: ");

        if (choice == 0) {
            user->saveToFile();
            std::cout << "Progress saved. Stay strong!\n";
            break;
        }

        switch (choice) {
            case 1: menuNewWorkout(*user);                                        break;
            case 2: user->listWorkouts(false);                                    break;
            case 3: ProgressTracker::printAllPRs(user->getPRs());                break;
            case 4: {
                std::string exName = inputLine("Enter exercise name for chart: ");
                ProgressTracker::volumeProgressChart(user->getWorkouts(), exName);
                break;
            }
            case 5: {
                std::string date = inputLine("Enter start date (YYYY-MM-DD): ");
                ProgressTracker::weeklySummary(user->getWorkouts(), date);
                break;
            }
            case 6: menuBodyWeight(*user);       break;
            case 7: menuMuscleFrequency(*user);  break;
            case 8: menuCalc1RM();               break;
            case 9: menuTemplates(*user);        break;
            default: std::cout << "Invalid choice.\n";
        }
    }

    std::cout << "Stay consistent! Goodbye.\n";
    delete user;
    return 0;
}
