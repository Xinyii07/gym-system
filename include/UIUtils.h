#pragma once
#include <string>
#include <vector>

// ================= UI RENDERING & BUFFER UTILITIES =================
void clearScreen();
int getLeftMargin();
void printBoxBorder();
void printBoxDivider();
void printBoxLine(const std::string& text, const std::string& align = "left", int indent = 4);
void displayAppHeader(const std::string& subtitle);
void pauseScreen();

// Shared helpers
std::string getCurrentDateStr();
bool isValidEmail(const std::string& email);
bool isValidPhone(const std::string& phone);
bool isValidSpecialization(const std::string& text); // letters/spaces/'&' only, non-empty
bool isValidExperience(const std::string& text);      // digits only, 1-2 characters (0-99 years)

// Reads a line of input without echoing it to the screen (prints '*' per
// character instead). Used for password prompts.
std::string readMaskedInput();

// Sets the role label shown top-right of every screen after login (e.g.
// "ADMIN" or "CUSTOMER (M1001)"). Pass "" to clear it (e.g. on logout).
void setCurrentRoleLabel(const std::string& label);

// Interactive arrow-key picker (Up/Down to move, Enter to select, Esc to
// cancel). Returns the selected index, or -1 if canceled or the list is
// empty. Falls back to a plain numbered prompt on non-Windows builds.
int selectFromList(const std::string& title, const std::vector<std::string>& items);
