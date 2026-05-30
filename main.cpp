#include "User.h"
#include "Menus.h"
#include "ProgressTracker.h"
#include <windows.h>
#include <iostream>
#include <iomanip>
#include <string>

// ── Box-drawing strings (multi-byte UTF-8, must be string literals not char) ──
// Each of these is a single box-drawing character encoded as a string.
#define BOX_TL  "\xe2\x95\x94"   // ╔
#define BOX_TR  "\xe2\x95\x97"   // ╗
#define BOX_BL  "\xe2\x95\x9a"   // ╚
#define BOX_BR  "\xe2\x95\x9d"   // ╝
#define BOX_H   "\xe2\x95\x90"   // ═  (double horizontal)
#define BOX_V   "\xe2\x95\x91"   // ║  (double vertical)
#define BOX_ML  "\xe2\x95\x9f"   // ╟  (left mid-join)
#define BOX_MR  "\xe2\x95\xa2"   // ╢  (right mid-join)
#define BOX_MH  "\xe2\x95\x80"   // ─  (single horizontal, for dividers)

// Repeat a UTF-8 box-drawing string n times
static std::string rep(const char* s, int n) {
    std::string out;
    out.reserve(n * 3);
    for (int i = 0; i < n; i++) out += s;
    return out;
}

static void printBanner() {
    std::cout << "\n"
              << "  \033[1;36m" BOX_TL << rep(BOX_H, 50) << BOX_TR "\033[0m\n"
              << "  \033[1;36m" BOX_V  "\033[0m"
              << "\033[1m" "        WORKOUT TRACKER  v2.0                   " "\033[0m"
              << "\033[1;36m" BOX_V "\033[0m\n"
              << "  \033[1;36m" BOX_V  "\033[0m"
              << "\033[2m" "        Log. Track. Progress. Dominate.          " "\033[0m"
              << "\033[1;36m" BOX_V "\033[0m\n"
              << "  \033[1;36m" BOX_BL << rep(BOX_H, 50) << BOX_BR "\033[0m\n\n";
}

static void printMainMenu(const User& user) {
    std::cout << "\n"
              << "  \033[1;36m" BOX_TL << rep(BOX_H, 34) << BOX_TR "\033[0m\n"
              << "  \033[1;36m" BOX_V  "\033[0m"
              << "\033[1m  MAIN MENU                          \033[0m"
              << "\033[1;36m" BOX_V "\033[0m\n"
              << "  \033[1;36m" BOX_ML << rep(BOX_MH, 34) << BOX_MR "\033[0m\n";

    auto row = [](int n, const char* icon, const char* label) {
        std::cout << "  \033[1;36m" BOX_V "\033[0m  "
                  << "\033[36m[" << n << "]\033[0m "
                  << icon << " "
                  << "\033[1m" << std::left << std::setw(26) << label << "\033[0m"
                  << "\033[1;36m" BOX_V "\033[0m\n";
    };

    row(1, "\xf0\x9f\x8f\x8b", "Log New Workout");
    row(2, "\xf0\x9f\x93\x96", "Workout History");
    row(3, "\xf0\x9f\x8f\x86", "Personal Records");
    row(4, "\xf0\x9f\x93\x8a", "Progress Charts");
    row(5, "\xf0\x9f\x93\x85", "Weekly Summary");
    row(6, "\xe2\x9a\x96 ", "Body Weight Tracker");
    row(7, "\xf0\x9f\x92\xaa", "Strength Standards");
    row(8, "\xf0\x9f\xa7\xae", "1RM Calculator");
    row(9, "\xf0\x9f\x93\x8b", "Workout Templates");

    std::cout << "  \033[1;36m" BOX_ML << rep(BOX_MH, 34) << BOX_MR "\033[0m\n";

    // Stats bar
    const auto& wks = user.getWorkouts();
    std::string stat = std::to_string(wks.size()) + " workouts logged";
    std::cout << "  \033[1;36m" BOX_V "\033[0m  "
              << "\033[2m" << std::left << std::setw(30) << stat << "\033[0m"
              << "\033[1;36m" BOX_V "\033[0m\n";

    std::cout << "  \033[1;36m" BOX_ML << rep(BOX_MH, 34) << BOX_MR "\033[0m\n";
    row(0, "\xf0\x9f\x9a\xaa", "Save & Exit");
    std::cout << "  \033[1;36m" BOX_BL << rep(BOX_H, 34) << BOX_BR "\033[0m\n\n";
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    // Enable ANSI escape codes on Windows 10+
    // 0x0004 == ENABLE_VIRTUAL_TERMINAL_PROCESSING (avoids missing header on some toolchains)
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    if (GetConsoleMode(hOut, &dwMode))
        SetConsoleMode(hOut, dwMode | 0x0004);

    User* user = User::loadFromFile();

    printBanner();

    if (!user) {
        std::cout << "  \033[1mWelcome! Let's set up your profile.\033[0m\n\n";
        std::string uname   = inputLine("Your name        : ");
        int         uage    = inputInt("Age              : ");
        double      uweight = inputDouble("Body weight (kg) : ");
        user = new User(uname, uage, uweight);
        std::cout << "\n  \033[1;32m\xe2\x9c\x94  Profile created. Let's get to work!\033[0m\n";
    } else {
        std::cout << "  \033[1mWelcome back, " << user->getName() << "!\033[0m\n";
        const auto& wks = user->getWorkouts();
        if (!wks.empty()) {
            std::cout << "  \033[2mLast session: " << wks.back()->getDate()
                      << "  |  " << wks.size() << " total workouts\033[0m\n";
        }
    }

    while (true) {
        printMainMenu(*user);
        int choice = inputInt("Select: ");

        if (choice == 0) {
            user->saveToFile();
            std::cout << "\n  \033[1;32m\xe2\x9c\x94  Progress saved.\033[0m  "
                      << "\033[2mStay consistent. See you next time!\033[0m\n\n";
            break;
        }

        switch (choice) {
            case 1: menuNewWorkout(*user);      break;
            case 2: menuWorkoutHistory(*user);  break;
            case 3: menuEditPRs(*user);         break;
            case 4: {
                // Progress charts submenu
                std::cout << "\n  \033[1;36m\xe2\x96\xba Progress Charts\033[0m\n"
                          << "  " << std::string(72, '-') << "\n"
                          << "    \033[36m[1]\033[0m Volume progress  (per exercise)\n"
                          << "    \033[36m[2]\033[0m Volume by muscle group\n"
                          << "    \033[36m[3]\033[0m Workout heatmap\n"
                          << "    \033[36m[4]\033[0m Body weight chart\n"
                          << "    \033[36m[5]\033[0m Muscle frequency\n"
                          << "    \033[36m[0]\033[0m Back\n";
                int sub = inputInt("> ");
                switch (sub) {
                    case 1: {
                        std::string ex = inputLine("Exercise name: ");
                        ProgressTracker::volumeProgressChart(user->getWorkouts(), ex);
                        break;
                    }
                    case 2: ProgressTracker::muscleVolumeChart(user->getWorkouts()); break;
                    case 3: ProgressTracker::workoutHeatmap(user->getWorkouts());    break;
                    case 4: ProgressTracker::bodyWeightChart(user->getBWLog());      break;
                    case 5: menuMuscleFrequency(*user);                              break;
                    default: break;
                }
                break;
            }
            case 5: {
                std::string date = inputLine("Week start date (YYYY-MM-DD): ");
                ProgressTracker::weeklySummary(user->getWorkouts(), date);
                break;
            }
            case 6: menuBodyWeight(*user); break;
            case 7: {
                ProgressTracker::strengthStandards(
                    user->getPRs(), user->getWeight());
                break;
            }
            case 8: menuCalc1RM();         break;
            case 9: menuTemplates(*user);  break;
            default:
                std::cout << "  \033[31m\xe2\x9c\x96  Invalid choice. Pick 0-9.\033[0m\n";
        }
    }

    delete user;
    return 0;
}