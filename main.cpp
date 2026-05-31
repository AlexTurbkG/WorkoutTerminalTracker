#include "User.h"
#include "Menus.h"
#include "ProgressTracker.h"
#include <windows.h>
#include <iostream>
#include <iomanip>
#include <string>

// ── Box-drawing macros ───────────────────────────────────────
#define BOX_TL  "\xe2\x95\x94"   // ╔
#define BOX_TR  "\xe2\x95\x97"   // ╗
#define BOX_BL  "\xe2\x95\x9a"   // ╚
#define BOX_BR  "\xe2\x95\x9d"   // ╝
#define BOX_H   "\xe2\x95\x90"   // ═
#define BOX_V   "\xe2\x95\x91"   // ║
#define BOX_ML  "\xe2\x95\xa0"   // ╠
#define BOX_MR  "\xe2\x95\xa3"   // ╣
#define BOX_MH  "\xe2\x95\x90"   // ═

static std::string rep(const char* s, int n) {
    std::string out; out.reserve(n * 3);
    for (int i = 0; i < n; i++) out += s;
    return out;
}

// ── Shared constants (must match Menus.cpp's C:: namespace) ──
// We just use the raw codes here to avoid a cross-TU dependency.
#define CY  "\033[1;36m"   // bold cyan
#define RST "\033[0m"
#define BLD "\033[1m"
#define DIM "\033[2m"
#define GRN "\033[1;32m"
#define RED "\033[31m"

static const int INNER = 34;

// ── Banner ───────────────────────────────────────────────────
static void printBanner() {
    std::cout << "\n"
              << "  " RED BOX_TL << rep(BOX_H, INNER) << BOX_TR RST "\n"
              << "  " RED BOX_V RST
              << BLD "  WORKOUT TRACKER  v2.0           " RST
              << RED BOX_V RST "\n"
              << "  " RED BOX_V RST
              << DIM "  Log. Track. Progress. Dominate. " RST
              << RED BOX_V RST "\n"
              << "  " RED BOX_BL << rep(BOX_H, INNER) << BOX_BR RST "\n\n";
}

// ── Main menu with Logout ────────────────────────────────────
// This function is called from Menus.cpp via extern declaration.
void printMainMenuFull(const User& user) {
    // Row: BOX_V(1) + " [n] "(5) + label(29) + BOX_V(1) — label must be exactly 29 chars
    auto row = [](int n, const char* label29) {
        // number "[10]" is 4 chars, "[n]" is 3 — pad number field to 4 always
        std::string num = "[" + std::to_string(n) + "]";
        while ((int)num.size() < 4) num += " ";
        std::cout << "  " CY BOX_V RST " "
                  << "\033[36m" << num << RST " "
                  << GRN BLD << label29 << RST
                  << CY BOX_V RST "\n";
    };

    // Stats line — exactly INNER chars
    std::string stat = " " + std::to_string(user.getWorkouts().size()) + " workouts  |  " + user.getName();
    while ((int)stat.size() < INNER) stat += ' ';
    stat = stat.substr(0, INNER);

    std::cout << "\n"
              << "  " CY BOX_TL << rep(BOX_H, INNER) << BOX_TR RST "\n"
              << "  " CY BOX_V RST
              << BLD "  MAIN MENU                       " RST
              << CY BOX_V RST "\n"
              << "  " CY BOX_ML << rep(BOX_MH, INNER) << BOX_MR RST "\n";

    //             "1234567890123456789012345678 9"  (29 chars)
    row( 1, " Log New Workout            ");
    row( 2, " Workout History            ");
    row( 3, " Personal Records           ");
    row( 4, " Progress Charts            ");
    row( 5, " Weekly Summary             ");
    row( 6, " Body Weight Tracker        ");
    row( 7, " Strength Standards         ");
    row( 8, " 1RM Calculator             ");
    row( 9, " Workout Templates          ");

    std::cout << "  " CY BOX_ML << rep(BOX_MH, INNER) << BOX_MR RST "\n";
    std::cout << "  " CY BOX_V RST
              << DIM << stat << RST
              << CY BOX_V RST "\n";
    std::cout << "  " CY BOX_ML << rep(BOX_MH, INNER) << BOX_MR RST "\n";

    row( 0, " Save & Exit                ");
    row(10, " Log Out                    ");

    std::cout << "  " CY BOX_BL << rep(BOX_H, INNER) << BOX_BR RST "\n\n";
}

// ── User-select screen ───────────────────────────────────────
// Returns a fully loaded User*, or nullptr if the user wants to quit.
static User* userSelectScreen() {
    printBanner();

    auto savedFiles = User::listSavedUsers();

    std::cout << "  " CY BOX_TL << rep(BOX_H, INNER) << BOX_TR RST "\n"
              << "  " CY BOX_V RST
              << BLD "  SELECT USER                     " RST
              << CY BOX_V RST "\n"
              << "  " CY BOX_ML << rep(BOX_MH, INNER) << BOX_MR RST "\n";

    if (savedFiles.empty()) {
        std::cout << "  " CY BOX_V RST
                  << DIM "  No saved users found.           " RST
                  << CY BOX_V RST "\n";
    } else {
        for (int i = 0; i < (int)savedFiles.size(); i++) {
            // Strip "user_" prefix and ".txt" suffix for display
            std::string display = savedFiles[i];
            if (display.size() > 5 && display.substr(0,5) == "user_")
                display = display.substr(5);
            if (display.size() > 4 && display.substr(display.size()-4) == ".txt")
                display = display.substr(0, display.size()-4);
            // Replace underscores back to spaces
            for (char& c : display) if (c == '_') c = ' ';

            // Pad to 29 chars for the box
            std::string line = "  [" + std::to_string(i+1) + "] " + display;
            while ((int)line.size() < INNER) line += ' ';
            line = line.substr(0, INNER);
            std::cout << "  " CY BOX_V RST "\033[36m" << line << RST
                      << CY BOX_V RST "\n";
        }
    }

    std::cout << "  " CY BOX_ML << rep(BOX_MH, INNER) << BOX_MR RST "\n";

    std::string newLine = "  [" + std::to_string(savedFiles.size()+1) + "] Create new user     ";
    while ((int)newLine.size() < INNER) newLine += ' ';
    newLine = newLine.substr(0, INNER);
    std::cout << "  " CY BOX_V RST "\033[36m" << newLine << RST << CY BOX_V RST "\n";

    std::string exitLine = "  [0] Exit program           ";
    while ((int)exitLine.size() < INNER) exitLine += ' ';
    exitLine = exitLine.substr(0, INNER);
    std::cout << "  " CY BOX_V RST "\033[36m" << exitLine << RST << CY BOX_V RST "\n";

    std::cout << "  " CY BOX_BL << rep(BOX_H, INNER) << BOX_BR RST "\n\n";

    int choice = inputInt("Select: ");

    if (choice == 0) return nullptr;

    // ── Create new user ──────────────────────────────────────
    if (choice == (int)savedFiles.size() + 1) {
        std::cout << "\n  " BLD "Create New Profile" RST "\n"
                  << "  " << std::string(34, '-') << "\n";
        std::string uname   = inputLine("Name             : ");
        if (uname.empty()) { std::cout << "  " RED "Name cannot be empty.\n" RST; return nullptr; }
        int    uage    = inputInt("Age              : ");
        double uweight = inputDouble("Body weight (kg) : ");
        User* u = new User(uname, uage, uweight);
        u->saveToFile();  // create the file immediately
        std::cout << "\n  " GRN "\xe2\x9c\x94  Profile created for " << uname << "!" RST "\n";
        return u;
    }

    // ── Load existing user ───────────────────────────────────
    if (choice >= 1 && choice <= (int)savedFiles.size()) {
        User* u = User::loadFromFile(savedFiles[choice - 1]);
        if (!u) {
            std::cout << "  " RED "Failed to load user file.\n" RST;
            return nullptr;
        }
        std::cout << "\n  " BLD "Welcome back, " << u->getName() << "!" RST "\n";
        const auto& wks = u->getWorkouts();
        if (!wks.empty())
            std::cout << "  " DIM "Last session: " << wks.back()->getDate()
                      << "  |  " << wks.size() << " total workouts" RST "\n";
        return u;
    }

    std::cout << "  " RED "Invalid choice." RST "\n";
    return nullptr;
}

// ── main ─────────────────────────────────────────────────────
int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    if (GetConsoleMode(hOut, &dwMode))
        SetConsoleMode(hOut, dwMode | 0x0004);

    // Outer loop: keep returning to user-select after logout
    while (true) {
        User* user = userSelectScreen();

        if (!user) {
            // User chose Exit (0) or invalid input
            std::cout << "\n  " DIM "Goodbye!\n\n" RST;
            break;
        }

        // Inner loop: the actual session
        SessionResult result = runUserSession(*user);
        delete user;

        if (result == SessionResult::Exit) {
            std::cout << "\n  " DIM "Goodbye!\n\n" RST;
            break;
        }
        // SessionResult::Logout -> loop back to user-select
    }

    return 0;
}