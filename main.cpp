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
#define BOX_ML  "\xe2\x95\xa0"   // ╠  (left mid-join, double-double)
#define BOX_MR  "\xe2\x95\xa3"   // ╣  (right mid-join, double-double)
#define BOX_MH  "\xe2\x95\x90"   // ═  (double horizontal, matches top/bottom)

// Repeat a UTF-8 box-drawing string n times
static std::string rep(const char* s, int n) {
    std::string out;
    out.reserve(n * 3);
    for (int i = 0; i < n; i++) out += s;
    return out;
}

static void printBanner() {
    // Inner width = 34, same as menu, so both boxes align.
    // Content strings must be exactly 34 visible chars each.
    std::cout << "\n"
              << "  \033[1;36m" BOX_TL << rep(BOX_H, 34) << BOX_TR "\033[0m\n"
              << "  \033[1;36m" BOX_V "\033[0m"
              << "\033[1m" "  WORKOUT TRACKER  v2.0           " "\033[0m"
              << "\033[1;36m" BOX_V "\033[0m\n"
              << "  \033[1;36m" BOX_V "\033[0m"
              << "\033[2m" "  Log. Track. Progress. Dominate. " "\033[0m"
              << "\033[1;36m" BOX_V "\033[0m\n"
              << "  \033[1;36m" BOX_BL << rep(BOX_H, 34) << BOX_BR "\033[0m\n\n";
}

static void printMainMenu(const User& user) {
    // Box inner width = 34 cols exactly.
    // Row anatomy: BOX_V(1) + " [n] "(5) + label(29) + BOX_V(1) = 36 total visible
    // label field = 34 - 5 = 29 chars — every label string below is exactly 29 chars.
    const int INNER = 34;

    // Each label is manually padded to exactly 29 visible chars.
    // Format: "  Text" + spaces to reach 29.
    auto row = [](int n, const char* label29) {
        std::cout << "  \033[1;36m" BOX_V "\033[0m"
                  << " \033[36m[" << n << "]\033[0m "
                  << "\033[1m" << label29 << "\033[0m"
                  << "\033[1;36m" BOX_V "\033[0m\n";
    };

    std::cout << "\n"
              << "  \033[1;36m" BOX_TL << rep(BOX_H, INNER) << BOX_TR "\033[0m\n"
              // Title row: BOX_V + 34 chars + BOX_V
              << "  \033[1;36m" BOX_V "\033[0m"
              << "\033[1m  MAIN MENU                       \033[0m"
              << "\033[1;36m" BOX_V "\033[0m\n"
              << "  \033[1;36m" BOX_ML << rep(BOX_MH, INNER) << BOX_MR "\033[0m\n";

    //                              1234567890123456789012345678 9
    row(1, "  Log New Workout            ");
    row(2, "  Workout History            ");
    row(3, "  Personal Records           ");
    row(4, "  Progress Charts            ");
    row(5, "  Weekly Summary             ");
    row(6, "  Body Weight Tracker        ");
    row(7, "  Strength Standards         ");
    row(8, "  1RM Calculator             ");
    row(9, "  Workout Templates          ");

    std::cout << "  \033[1;36m" BOX_ML << rep(BOX_MH, INNER) << BOX_MR "\033[0m\n";

    // Stats row: BOX_V + 34 chars + BOX_V
    // Build stat string, then pad/truncate to exactly 34 chars
    std::string statBase = " " + std::to_string(user.getWorkouts().size()) + " workouts logged";
    while ((int)statBase.size() < INNER) statBase += ' ';
    statBase = statBase.substr(0, INNER);
    std::cout << "  \033[1;36m" BOX_V "\033[0m"
              << "\033[2m" << statBase << "\033[0m"
              << "\033[1;36m" BOX_V "\033[0m\n";

    std::cout << "  \033[1;36m" BOX_ML << rep(BOX_MH, INNER) << BOX_MR "\033[0m\n";
    row(0, "  Save & Exit                ");
    std::cout << "  \033[1;36m" BOX_BL << rep(BOX_H, INNER) << BOX_BR "\033[0m\n\n";
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