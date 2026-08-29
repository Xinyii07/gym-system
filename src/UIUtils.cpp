#include "UIUtils.h"
#include "Config.h"
#include <iostream>
#include <algorithm>
#include <limits>
#include <ctime>
#include <cctype>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#endif

using namespace std;

// Helper function to get current system date (YYYY-MM-DD)
string getCurrentDateStr() {
    time_t now = time(0);
    tm ltm;
#ifdef _WIN32
    localtime_s(&ltm, &now);
#else
    localtime_r(&now, &ltm);
#endif
    char buf[11];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d", 1900 + ltm.tm_year, 1 + ltm.tm_mon, ltm.tm_mday);
    return string(buf);
}

// Helper function to validate email format
bool isValidEmail(const string& email) {
    size_t atPos = email.find('@');
    size_t dotPos = email.rfind('.');
    return (atPos != string::npos && dotPos != string::npos && atPos < dotPos && atPos > 0 && dotPos < email.length() - 1);
}

// Helper function to validate a phone number: digits only, 10-11 digits long
bool isValidPhone(const string& phone) {
    if (phone.length() < 10 || phone.length() > 11) return false;
    for (char c : phone) {
        if (!isdigit(static_cast<unsigned char>(c))) return false;
    }
    return true;
}

// Words only: letters, spaces, and '&' (e.g. "Yoga & Fitness"). No digits/symbols.
bool isValidSpecialization(const string& text) {
    if (text.empty()) return false;
    for (char c : text) {
        if (!isalpha(static_cast<unsigned char>(c)) && c != ' ' && c != '&') return false;
    }
    return true;
}

// Numbers only, 1-2 digits (0-99 years)
bool isValidExperience(const string& text) {
    if (text.empty() || text.length() > 2) return false;
    for (char c : text) {
        if (!isdigit(static_cast<unsigned char>(c))) return false;
    }
    return true;
}

string readMaskedInput() {
    string input;
#ifdef _WIN32
    char ch;
    while ((ch = _getch()) != '\r' && ch != '\n') {
        if (ch == '\b') {
            if (!input.empty()) {
                input.pop_back();
                cout << "\b \b";
            }
        }
        else {
            input += ch;
            cout << '*';
        }
    }
    cout << "\n";
#else
    cin.ignore((numeric_limits<streamsize>::max)(), '\n');
    getline(cin, input);
#endif
    return input;
}

namespace {
    string g_roleLabel;
}

void setCurrentRoleLabel(const string& label) {
    g_roleLabel = label;
}

int selectFromList(const string& title, const vector<string>& items) {
    if (items.empty()) return -1;

#ifdef _WIN32
    int selected = 0;
    while (true) {
        clearScreen();
        displayAppHeader(title);
        for (size_t i = 0; i < items.size(); ++i) {
            string marker = (static_cast<int>(i) == selected) ? "> " : "  ";
            printBoxLine(marker + items[i], "left", 6);
        }
        printBoxDivider();
        printBoxLine("Up/Down = move | Enter = select | Esc = cancel", "center");
        printBoxBorder();

        int key = _getch();
        if (key == 0 || key == 224) { // extended key prefix (arrows, F-keys, ...)
            int key2 = _getch();
            if (key2 == 72) { // Up
                selected = (selected - 1 + (int)items.size()) % (int)items.size();
            }
            else if (key2 == 80) { // Down
                selected = (selected + 1) % (int)items.size();
            }
        }
        else if (key == '\r' || key == '\n') {
            return selected;
        }
        else if (key == 27) { // Esc
            return -1;
        }
    }
#else
    clearScreen();
    displayAppHeader(title);
    for (size_t i = 0; i < items.size(); ++i) {
        printBoxLine("[" + to_string(i + 1) + "] " + items[i], "left", 6);
    }
    printBoxLine("[0] Cancel", "left", 6);
    printBoxBorder();

    int choice;
    cout << "\n" << string(getLeftMargin() + 6, ' ') << "Select: ";
    if (!(cin >> choice) || choice < 1 || choice > (int)items.size()) return -1;
    return choice - 1;
#endif
}

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

int getLeftMargin() {
    int winWidth = 115;
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        winWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    }
#endif
    return max(0, (winWidth - BOX_WIDTH) / 2);
}

void printBoxBorder() {
    cout << string(getLeftMargin(), ' ') << "+" << string(INNER_WIDTH, '-') << "+\n";
}

void printBoxDivider() {
    cout << string(getLeftMargin(), ' ') << "|" << string(INNER_WIDTH, '-') << "|\n";
}

void printBoxLine(const string& text, const string& align, int indent) {
    string content = "";
    int len = (int)text.length();

    if (align == "center") {
        int leftPad = max(0, (INNER_WIDTH - len) / 2);
        int rightPad = max(0, INNER_WIDTH - len - leftPad);
        content = string(leftPad, ' ') + text + string(rightPad, ' ');
    }
    else if (align == "right") {
        int rightPad = indent;
        int leftPad = max(0, INNER_WIDTH - len - rightPad);
        content = string(leftPad, ' ') + text + string(rightPad, ' ');
    }
    else {
        int leftPad = indent;
        int rightPad = max(0, INNER_WIDTH - len - leftPad);
        content = string(leftPad, ' ') + text + string(rightPad, ' ');
    }

    cout << string(getLeftMargin(), ' ') << "|" << content << "|\n";
}

void displayAppHeader(const string& subtitle) {
    printBoxBorder();
    if (!g_roleLabel.empty()) {
        printBoxLine("Role: " + g_roleLabel, "right", 2);
        printBoxDivider();
    }
    printBoxLine("  ____   ___  __   __  _____ _ _                     ", "center");
    printBoxLine(" / ___| |_  | \\ \\ / / |  ___(_) |_ _ __   ___  ___ ___ ", "center");
    printBoxLine(" \\___ \\   | |  \\ V /  | |_  | | __| '_ \\ / _ \\/ __/ __|", "center");
    printBoxLine("  ___) |  | |   | |   |  _| | | |_| | | |  __/\\__ \\__ \\", "center");
    printBoxLine(" |____/__/ |   |_|   |_|   |_|\\__|_| |_|\\___||___/___/", "center");
    printBoxLine(" ", "center");
    printBoxLine("SJY FITNESS - SERVICE MANAGEMENT SYSTEM", "center");
    printBoxLine("Your Health, Our Passion", "center");
    printBoxDivider();
    printBoxLine(">> " + subtitle + " <<", "center");
    printBoxDivider();
}

void pauseScreen() {
    cout << "\n" << string(getLeftMargin(), ' ') << ">> Press Enter to continue...";
    cin.clear();
    cin.ignore((numeric_limits<streamsize>::max)(), '\n');
    cin.get();
}
