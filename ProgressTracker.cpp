#include "ProgressTracker.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <ctime>

// ============================================================
// STYLE  (mirrors Menus.cpp — same look everywhere)
// ============================================================
namespace C {
    extern const char* RESET;
    extern const char* BOLD;
    extern const char* DIM;
    extern const char* GREEN;
    extern const char* YELLOW;
    extern const char* CYAN;
    extern const char* RED;
    extern const char* MAGENTA;
    extern const char* BGREEN;
    extern const char* BCYAN;
    extern const char* BYELLOW;
}

static const int COL = 72;

static void hr(char ch = '-') {
    std::cout << "  " << std::string(COL, ch) << "\n";
}
static void header(const std::string& t) {
    std::cout << "\n";
    hr('=');
    int pad = (COL - (int)t.size()) / 2;
    std::cout << "  " << std::string(std::max(pad,0), ' ')
              << C::BOLD << C::CYAN << t << C::RESET << "\n";
    hr('=');
}
static void subheader(const std::string& t) {
    std::cout << "\n  " << C::BOLD << C::YELLOW
              << "\xe2\x96\xba " << t << C::RESET << "\n";
    hr();
}
static void info(const std::string& m) {
    std::cout << "  " << C::CYAN << "\xe2\x84\xb9  " << m << C::RESET << "\n";
}
static void warn(const std::string& m) {
    std::cout << "  " << C::YELLOW << "\xe2\x9a\xa0  " << m << C::RESET << "\n";
}

// ── Bar drawing ──────────────────────────────────────────────
// Renders a horizontal bar using block characters with colour tiers.
static void drawBar(double value, double maxValue, int width,
                    const char* highCol  = "\033[32m",   // green
                    const char* midCol   = "\033[33m",   // yellow
                    const char* lowCol   = "\033[31m")   // red
{
    int filled = (maxValue > 0) ? (int)(value / maxValue * width) : 0;
    filled = std::max(0, std::min(filled, width));

    double ratio = (maxValue > 0) ? value / maxValue : 0;
    const char* col = (ratio >= 0.66) ? highCol :
                      (ratio >= 0.33) ? midCol  : lowCol;

    std::cout << col;
    for (int i = 0; i < filled; i++)       std::cout << "\xe2\x96\x88"; // █
    std::cout << C::DIM;
    for (int i = filled; i < width; i++)   std::cout << "\xe2\x96\x91"; // ░
    std::cout << C::RESET;
}

// ── Date helpers ─────────────────────────────────────────────
// Parse "YYYY-MM-DD" into y/m/d. Returns false on bad format.
static bool parseDate(const std::string& s, int& y, int& mo, int& d) {
    if (s.size() != 10 || s[4] != '-' || s[7] != '-') return false;
    try {
        y  = std::stoi(s.substr(0, 4));
        mo = std::stoi(s.substr(5, 2));
        d  = std::stoi(s.substr(8, 2));
        return true;
    } catch (...) { return false; }
}

static std::string formatDate(int y, int m, int d) {
    char buf[11];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d", y, m, d);
    return buf;
}

// Day of week (0=Sun … 6=Sat) via Tomohiko Sakamoto
static int dayOfWeek(int y, int m, int d) {
    static int t[] = {0,3,2,5,0,3,5,1,4,6,2,4};
    if (m < 3) y--;
    return (y + y/4 - y/100 + y/400 + t[m-1] + d) % 7;
}

// Days in a month (rough — good enough for heatmap)
static int daysInMonth(int y, int m) {
    if (m == 2) return (y%4==0 && (y%100!=0 || y%400==0)) ? 29 : 28;
    if (m==4||m==6||m==9||m==11) return 30;
    return 31;
}

// ============================================================
// UPDATE PRs
// ============================================================
void ProgressTracker::updatePRs(const Workout& w,
                                 std::map<std::string, PersonalRecord>& prs) {
    for (const auto* ex : w.getExercises()) {
        auto& pr = prs[ex->getName()];
        bool updated = false;
        for (const auto& s : ex->getSets()) {
            if (s.getWeight() > pr.bestWeight) { pr.bestWeight = s.getWeight(); updated = true; }
            if (s.getReps()   > pr.bestReps)   { pr.bestReps   = s.getReps();   updated = true; }
            if (s.getVolume() > pr.bestVolume) { pr.bestVolume = s.getVolume(); updated = true; }
            if (s.calc1RM()   > pr.best1RM)    { pr.best1RM    = s.calc1RM();   updated = true; }
        }
        if (updated) pr.date = w.getDate();
    }
}

// ============================================================
// PRINT ALL PRs
// ============================================================
void ProgressTracker::printAllPRs(
    const std::map<std::string, PersonalRecord>& prs)
{
    header("Personal Records");
    if (prs.empty()) { info("No PRs yet. Log some workouts!"); return; }

    // Column widths
    const int W0 = 28, W1 = 12, W2 = 10, W3 = 14, W4 = 12, W5 = 12;
    std::cout << "  " << C::BOLD << std::left
              << std::setw(W0) << "Exercise"
              << std::setw(W1) << "Weight"
              << std::setw(W2) << "Reps"
              << std::setw(W3) << "Volume"
              << std::setw(W4) << "Est. 1RM"
              << "Date"
              << C::RESET << "\n";
    hr();

    // Sort by exercise name
    std::vector<std::pair<std::string, PersonalRecord>> sorted(prs.begin(), prs.end());
    std::sort(sorted.begin(), sorted.end(),
        [](const auto& a, const auto& b){ return a.first < b.first; });

    int row = 0;
    for (const auto& kv : sorted) {
        const auto& pr = kv.second;
        const char* rowDim = (row++ % 2 == 1) ? C::DIM : "";

        auto fmtKg = [](double v) {
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(1) << v << " kg";
            return ss.str();
        };

        std::cout << "  " << rowDim << std::left
                  << std::setw(W0) << kv.first
                  << std::setw(W1) << fmtKg(pr.bestWeight)
                  << std::setw(W2) << pr.bestReps
                  << std::setw(W3) << fmtKg(pr.bestVolume)
                  << std::setw(W4) << fmtKg(pr.best1RM)
                  << (pr.date.empty() ? "-" : pr.date)
                  << C::RESET << "\n";
    }
    hr();
    std::cout << "  " << C::DIM << sorted.size() << " exercises tracked\n" << C::RESET;
}

// ============================================================
// VOLUME PROGRESS CHART  (per exercise, over time)
// ============================================================
void ProgressTracker::volumeProgressChart(
    const std::vector<Workout*>& workouts,
    const std::string& exerciseName)
{
    header("Volume Progress — " + exerciseName);

    std::vector<std::pair<std::string, double>> data;
    for (const auto* w : workouts)
        for (const auto* ex : w->getExercises())
            if (ex->getName() == exerciseName && !ex->getSets().empty())
                data.push_back({w->getDate(), ex->getTotalVolume()});

    if (data.empty()) {
        info("No data found for \"" + exerciseName + "\".");
        return;
    }

    double maxVol = 0, totalVol = 0;
    for (const auto& d : data) { maxVol = std::max(maxVol, d.second); totalVol += d.second; }
    double avgVol = totalVol / data.size();

    const int BAR = 32;
    const int DATE_W = 12;

    std::cout << "  " << C::BOLD << std::left
              << std::setw(DATE_W) << "Date"
              << "  " << std::setw(BAR) << "Volume"
              << "  kg\n" << C::RESET;
    hr();

    double prevVol = -1;
    for (const auto& kv : data) {
        // Trend arrow
        const char* arrow = "";
        if (prevVol >= 0) {
            if      (kv.second > prevVol * 1.01) arrow = "\033[32m \xe2\x86\x91\033[0m";  // ↑ green
            else if (kv.second < prevVol * 0.99) arrow = "\033[31m \xe2\x86\x93\033[0m";  // ↓ red
            else                                  arrow = "\033[2m \xe2\x86\x92\033[0m";   // → dim
        }
        prevVol = kv.second;

        std::cout << "  " << std::left << std::setw(DATE_W) << kv.first << "  ";
        drawBar(kv.second, maxVol, BAR);
        std::cout << "  " << C::BOLD
                  << std::fixed << std::setprecision(0) << kv.second
                  << C::RESET << " kg" << arrow << C::RESET << "\n";
    }
    hr();

    // Summary stats row
    std::cout << "  " << C::DIM
              << "Sessions: " << data.size()
              << "   Peak: " << std::fixed << std::setprecision(0) << maxVol << " kg"
              << "   Avg: "  << std::fixed << std::setprecision(0) << avgVol << " kg"
              << C::RESET << "\n";
}

// ============================================================
// MUSCLE VOLUME CHART  (total volume per muscle group)
// ============================================================
void ProgressTracker::muscleVolumeChart(const std::vector<Workout*>& workouts) {
    header("Volume by Muscle Group");

    if (workouts.empty()) { info("No workouts logged yet."); return; }

    // Accumulate volume per muscle group
    std::map<std::string, double> vol;
    std::map<std::string, int>    sessions;
    for (const auto* w : workouts) {
        for (const auto* ex : w->getExercises()) {
            vol[ex->getMuscleGroup()]      += ex->getTotalVolume();
            sessions[ex->getMuscleGroup()] += 1;
        }
    }

    // Sort by volume descending
    std::vector<std::pair<std::string, double>> sorted(vol.begin(), vol.end());
    std::sort(sorted.begin(), sorted.end(),
        [](const auto& a, const auto& b){ return a.second > b.second; });

    double maxVol = sorted.front().second;
    const int BAR = 28;
    const int MG_W = 16;

    std::cout << "  " << C::BOLD << std::left
              << std::setw(MG_W) << "Muscle Group"
              << "  " << std::setw(BAR) << "Total Volume"
              << "  kg      Sets\n" << C::RESET;
    hr();

    double grandTotal = 0;
    for (const auto& kv : sorted) grandTotal += kv.second;

    int row = 0;
    for (const auto& kv : sorted) {
        const char* rowDim = (row++ % 2 == 1) ? C::DIM : "";
        double pct = (grandTotal > 0) ? kv.second / grandTotal * 100.0 : 0;

        std::cout << "  " << rowDim << std::left << std::setw(MG_W) << kv.first << "  ";
        drawBar(kv.second, maxVol, BAR);
        std::cout << "  " << C::BOLD
                  << std::fixed << std::setprecision(0) << std::setw(8) << kv.second
                  << C::RESET << rowDim
                  << std::setw(4) << sessions[kv.first]
                  << C::DIM << "  (" << std::fixed << std::setprecision(1) << pct << "%)"
                  << C::RESET << "\n";
    }
    hr();
    std::cout << "  " << C::DIM
              << "Total volume across all muscles: "
              << std::fixed << std::setprecision(0) << grandTotal << " kg"
              << C::RESET << "\n";

    // Imbalance warning — flag if any muscle is trained < 10% of the most trained
    for (const auto& kv : sorted) {
        if (kv.second < maxVol * 0.10 && kv.first != sorted.front().first)
            warn("Low volume on " + kv.first + " — consider adding more work.");
    }
}

// ============================================================
// BODY WEIGHT CHART
// ============================================================
void ProgressTracker::bodyWeightChart(const std::map<std::string, double>& bwLog) {
    header("Body Weight Progress");

    if (bwLog.empty()) { info("No body weight data logged yet."); return; }

    double minW = 1e9, maxW = 0, sum = 0;
    for (const auto& kv : bwLog) {
        minW = std::min(minW, kv.second);
        maxW = std::max(maxW, kv.second);
        sum += kv.second;
    }
    double avg   = sum / bwLog.size();
    double range = maxW - minW;

    const int BAR    = 28;
    const int DATE_W = 12;

    std::cout << "  " << C::BOLD << std::left
              << std::setw(DATE_W) << "Date"
              << "  " << std::setw(BAR) << "Weight"
              << "  kg\n" << C::RESET;
    hr();

    double prevW = -1;
    for (const auto& kv : bwLog) {
        // Bar filled proportional to range above minimum
        double fillVal = (range > 0) ? (kv.second - minW) : 0.5;
        double fillMax = (range > 0) ? range : 1.0;

        const char* arrow = "";
        if (prevW >= 0) {
            if      (kv.second < prevW - 0.05) arrow = "\033[32m \xe2\x86\x93\033[0m"; // ↓ losing = green
            else if (kv.second > prevW + 0.05) arrow = "\033[31m \xe2\x86\x91\033[0m"; // ↑ gaining = red
            else                                arrow = "\033[2m \xe2\x86\x92\033[0m";  // →
        }
        prevW = kv.second;

        std::cout << "  " << std::left << std::setw(DATE_W) << kv.first << "  ";
        // Use a neutral blue-ish colour for body weight
        drawBar(fillVal, fillMax, BAR, "\033[36m", "\033[36m", "\033[36m");
        std::cout << "  " << C::BOLD
                  << std::fixed << std::setprecision(1) << kv.second
                  << C::RESET << " kg" << arrow << C::RESET << "\n";
    }
    hr();
    std::cout << "  " << C::DIM
              << "Min: " << std::fixed << std::setprecision(1) << minW << " kg"
              << "   Max: " << maxW << " kg"
              << "   Avg: " << avg  << " kg"
              << "   Range: " << std::setprecision(1) << range << " kg"
              << C::RESET << "\n";
}

// ============================================================
// WORKOUT HEATMAP  (GitHub-style, last 16 weeks)
// ============================================================
void ProgressTracker::workoutHeatmap(const std::vector<Workout*>& workouts) {
    header("Workout Heatmap — Last 16 Weeks");

    // Build a set of worked-out dates -> workout count
    std::map<std::string, int> dayCount;
    for (const auto* w : workouts)
        dayCount[w->getDate()]++;

    // Find today
    time_t now = time(nullptr);
    tm* lt = localtime(&now);
    int todayY = lt->tm_year + 1900;
    int todayM = lt->tm_mon  + 1;
    int todayD = lt->tm_mday;

    // We show 16 weeks = 112 days, starting on the Sunday 112 days ago
    // Find that Sunday
    // Step back 111 days from today
    // Simple approach: compute an absolute day count, step back, find Sunday
    auto toDays = [](int y, int m, int d) -> int {
        // Days since a fixed epoch (Gregorian)
        int a = (14 - m) / 12;
        int yy = y + 4800 - a;
        int mm = m + 12*a - 3;
        return d + (153*mm+2)/5 + 365*yy + yy/4 - yy/100 + yy/400 - 32045;
    };
    auto fromDays = [](int jd, int& y, int& m, int& d) {
        int a = jd + 32044;
        int b = (4*a+3)/146097;
        int c = a - (146097*b)/4;
        int dd = (4*c+3)/1461;
        int e = c - (1461*dd)/4;
        int mm = (5*e+2)/153;
        d = e - (153*mm+2)/5 + 1;
        m = mm + 3 - 12*(mm/10);
        y = 100*b + dd - 4800 + mm/10;
    };

    int todayJD = toDays(todayY, todayM, todayD);
    // Find last Sunday on or before today
    int dow = dayOfWeek(todayY, todayM, todayD); // 0=Sun
    int lastSundayJD = todayJD - dow;
    // Start 15 full weeks before that Sunday (16 weeks total)
    int startJD = lastSundayJD - 15*7;

    const int WEEKS = 16;

    // Day-of-week labels (Mon offset since we label Mon-Sun visually)
    const char* dowLabels[] = {"Su","Mo","Tu","We","Th","Fr","Sa"};

    // Print month labels across the top
    std::cout << "  " << C::BOLD << "     "; // indent for row labels
    int prevMonth = -1;
    for (int w = 0; w < WEEKS; w++) {
        int jd = startJD + w*7;
        int y, m, d;
        fromDays(jd, y, m, d);
        if (m != prevMonth) {
            // Print month abbrev
            const char* months[] = {"","Jan","Feb","Mar","Apr","May","Jun",
                                    "Jul","Aug","Sep","Oct","Nov","Dec"};
            std::cout << C::CYAN << std::left << std::setw(6) << months[m] << C::RESET;
            prevMonth = m;
        } else {
            std::cout << "      ";
        }
    }
    std::cout << C::RESET << "\n";

    // Print rows: one per day of week
    for (int dow2 = 0; dow2 < 7; dow2++) {
        // Row label (Mon, Wed, Fri only to avoid clutter)
        if (dow2 == 1 || dow2 == 3 || dow2 == 5)
            std::cout << "  " << C::DIM << dowLabels[dow2] << C::RESET << "  ";
        else
            std::cout << "      ";

        for (int w = 0; w < WEEKS; w++) {
            int jd = startJD + w*7 + dow2;
            int y, m, d;
            fromDays(jd, y, m, d);
            std::string dateStr = formatDate(y, m, d);

            auto it = dayCount.find(dateStr);
            int count = (it != dayCount.end()) ? it->second : 0;

            // Future days
            if (jd > todayJD) {
                std::cout << C::DIM << "\xe2\x96\xa1\xe2\x96\xa1 " << C::RESET; // □ empty future
                continue;
            }

            // Colour by workout count
            if (count == 0) {
                std::cout << C::DIM << "\xe2\x96\xa1\xe2\x96\xa1 " << C::RESET;
            } else if (count == 1) {
                std::cout << "\033[32m" << "\xe2\x96\xa3\xe2\x96\xa3 " << C::RESET; // ▣ light
            } else if (count == 2) {
                std::cout << "\033[1;32m" << "\xe2\x96\xa3\xe2\x96\xa3 " << C::RESET; // brighter
            } else {
                std::cout << "\033[1;36m" << "\xe2\x96\xa3\xe2\x96\xa3 " << C::RESET; // cyan = very active
            }
        }
        std::cout << "\n";
    }

    // Legend
    std::cout << "\n  " << C::DIM
              << "Legend: "
              << C::RESET  << C::DIM   << "\xe2\x96\xa1\xe2\x96\xa1" << C::RESET << " none  "
              << "\033[32m"            << "\xe2\x96\xa3\xe2\x96\xa3" << C::RESET << " 1 workout  "
              << "\033[1;32m"          << "\xe2\x96\xa3\xe2\x96\xa3" << C::RESET << " 2 workouts  "
              << "\033[1;36m"          << "\xe2\x96\xa3\xe2\x96\xa3" << C::RESET << " 3+\n";

    // Stats
    int activeDays = 0, totalW = (int)workouts.size();
    for (const auto& kv : dayCount) activeDays++;
    hr();
    std::cout << "  " << C::DIM
              << "Active days: " << activeDays
              << "   Total workouts: " << totalW;
    if (activeDays > 0)
        std::cout << "   Avg per active day: "
                  << std::fixed << std::setprecision(1)
                  << (double)totalW / activeDays;
    std::cout << C::RESET << "\n";
}

// ============================================================
// WEEKLY SUMMARY
// ============================================================
void ProgressTracker::weeklySummary(const std::vector<Workout*>& workouts,
                                    const std::string& weekStart) {
    header("Weekly Summary — from " + weekStart);

    int    count = 0, totalSets = 0, totalDur = 0;
    double totalVol = 0;
    std::map<std::string, double> muscleVol;
    std::map<std::string, int>    moodCount;

    for (const auto* w : workouts) {
        if (w->getDate() < weekStart) continue;
        count++;
        totalVol += w->getTotalVolume();
        totalSets += w->getTotalSets();
        totalDur  += w->getDuration();
        moodCount[w->getMood()]++;
        for (const auto* ex : w->getExercises())
            muscleVol[ex->getMuscleGroup()] += ex->getTotalVolume();
    }

    if (count == 0) { info("No workouts found from " + weekStart + " onwards."); return; }

    // ── Summary stats ────────────────────────────────────────
    subheader("Stats");
    auto stat = [](const std::string& label, const std::string& value) {
        std::cout << "  " << C::BOLD << std::left << std::setw(24) << label
                  << C::RESET << C::CYAN << value << C::RESET << "\n";
    };
    stat("Workouts completed",  std::to_string(count));
    stat("Total time",          std::to_string(totalDur) + " min  ("
                                + std::to_string(totalDur / 60) + "h "
                                + std::to_string(totalDur % 60) + "m)");
    stat("Total sets",          std::to_string(totalSets));
    stat("Total volume",        [&]{
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(0) << totalVol << " kg";
        return ss.str();
    }());
    if (count > 0)
        stat("Avg volume/session", [&]{
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(0) << totalVol/count << " kg";
            return ss.str();
        }());

    // ── Mood breakdown ───────────────────────────────────────
    if (!moodCount.empty()) {
        subheader("Mood");
        for (const auto& kv : moodCount) {
            std::string bar(kv.second * 3, '\xe2');  // reuse simpler approach
            std::cout << "  " << std::left << std::setw(12) << kv.first
                      << " " << C::CYAN;
            for (int i = 0; i < kv.second; i++) std::cout << "\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88";
            std::cout << C::RESET << "  " << kv.second << "x\n";
        }
    }

    // ── Muscle volume breakdown ──────────────────────────────
    if (!muscleVol.empty()) {
        subheader("Volume by Muscle");

        std::vector<std::pair<std::string,double>> mv(muscleVol.begin(), muscleVol.end());
        std::sort(mv.begin(), mv.end(),
            [](const auto& a, const auto& b){ return a.second > b.second; });
        double maxV = mv.front().second;
        const int BAR = 22;

        for (const auto& kv : mv) {
            std::cout << "  " << std::left << std::setw(14) << kv.first << "  ";
            drawBar(kv.second, maxV, BAR);
            std::cout << "  " << C::DIM
                      << std::fixed << std::setprecision(0) << kv.second << " kg"
                      << C::RESET << "\n";
        }
    }
    hr();
}

// ============================================================
// STRENGTH STANDARDS
// ============================================================
void ProgressTracker::strengthStandards(
    const std::map<std::string, PersonalRecord>& prs,
    double bwKg)
{
    header("Strength Standards");

    if (bwKg <= 0) { warn("Set your bodyweight first (Body Weight Tracker)."); return; }

    // Standards table: 1RM as multiple of bodyweight (male, general population)
    // Sources: Symmetric Strength / ExRx general guidelines
    static const StrengthStandard standards[] = {
        // exercise                    beg    inter  adv    elite
        {"Bench Press (Barbell)",      0.50,  1.00,  1.50,  2.00},
        {"Squat (Barbell)",            0.75,  1.25,  1.75,  2.50},
        {"Deadlift (Barbell)",         1.00,  1.50,  2.00,  2.75},
        {"Overhead Press (Barbell)",   0.35,  0.65,  1.00,  1.35},
        {"Bent Over Row (Barbell)",    0.50,  0.85,  1.25,  1.65},
        {"Romanian Deadlift (Barbell)",0.75,  1.15,  1.60,  2.10},
        {"Front Squat",                0.55,  0.90,  1.30,  1.80},
        {"Incline Bench Press (Barbell)",0.40, 0.80, 1.25,  1.65},
        {"Hip Thrust (Barbell)",       0.80,  1.30,  1.90,  2.50},
        {"Close Grip Bench Press (Barbell)", 0.40, 0.80, 1.20, 1.60},
    };
    const int N = sizeof(standards) / sizeof(standards[0]);

    // Header
    std::cout << "  " << C::DIM << "Bodyweight: " << std::fixed << std::setprecision(1)
              << bwKg << " kg\n\n" << C::RESET;

    std::cout << "  " << C::BOLD << std::left
              << std::setw(32) << "Exercise"
              << std::setw(10) << "Your 1RM"
              << std::setw(14) << "Level"
              << "Progress\n" << C::RESET;
    hr();

    int found = 0;
    for (int i = 0; i < N; i++) {
        const auto& s = standards[i];
        auto it = prs.find(s.exercise);

        double oneRM = 0;
        if (it != prs.end()) oneRM = it->second.best1RM;

        double ratio = (bwKg > 0) ? oneRM / bwKg : 0;

        // Determine level
        const char* levelStr;
        const char* levelCol;
        double nextTarget;
        double prevTarget;

        if      (ratio >= s.elite)        { levelStr = "ELITE";        levelCol = "\033[1;36m"; nextTarget = -1;           prevTarget = s.advanced; }
        else if (ratio >= s.advanced)     { levelStr = "Advanced";     levelCol = "\033[1;32m"; nextTarget = s.elite;      prevTarget = s.advanced; }
        else if (ratio >= s.intermediate) { levelStr = "Intermediate"; levelCol = "\033[32m";   nextTarget = s.advanced;   prevTarget = s.intermediate; }
        else if (ratio >= s.beginner)     { levelStr = "Beginner";     levelCol = "\033[33m";   nextTarget = s.intermediate; prevTarget = s.beginner; }
        else                              { levelStr = "Untrained";    levelCol = "\033[31m";   nextTarget = s.beginner;   prevTarget = 0; }

        // Progress bar toward next level
        double barFill = 0;
        if (nextTarget < 0) {
            barFill = 1.0; // maxed out
        } else {
            double lo = prevTarget * bwKg;
            double hi = nextTarget * bwKg;
            barFill = (hi > lo) ? std::min(1.0, (oneRM - lo) / (hi - lo)) : 0;
            if (barFill < 0) barFill = 0;
        }

        std::cout << "  " << std::left << std::setw(32) << s.exercise;

        if (oneRM > 0) {
            std::ostringstream rmStr;
            rmStr << std::fixed << std::setprecision(1) << oneRM << " kg";
            std::cout << std::setw(10) << rmStr.str();
        } else {
            std::cout << C::DIM << std::setw(10) << "no data" << C::RESET;
        }

        std::cout << levelCol << std::setw(14) << levelStr << C::RESET;

        // Draw the progress bar (14 chars wide)
        const int PBAR = 14;
        int filled = (int)(barFill * PBAR);
        std::cout << levelCol;
        for (int j = 0; j < filled; j++)        std::cout << "\xe2\x96\x88";
        std::cout << C::DIM;
        for (int j = filled; j < PBAR; j++)     std::cout << "\xe2\x96\x91";
        std::cout << C::RESET;

        // Show what target to hit next
        if (nextTarget > 0 && oneRM > 0) {
            double needed = nextTarget * bwKg;
            std::cout << C::DIM << "  \xe2\x86\x92 "
                      << std::fixed << std::setprecision(1) << needed << " kg";
        } else if (oneRM <= 0) {
            std::cout << C::DIM << "  log a set to start";
        } else {
            std::cout << C::DIM << "  \xe2\x9c\x94 maxed!";
        }
        std::cout << C::RESET << "\n";
        found++;
    }
    hr();

    std::cout << "  " << C::DIM
              << "Standards are 1RM multiples of bodyweight (general male population).\n"
              << "  Ratios differ by gender, age, and build — use as a guide, not a verdict.\n"
              << C::RESET;
}