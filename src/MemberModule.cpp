#include "MemberModule.h"
#include "UIUtils.h"
#include "Config.h"
#include "DataStore.h"
#include <iostream>
#include <vector>

#ifdef _WIN32
#include <conio.h>
#endif

using namespace std;

// Auto-generate the next "M####" member ID, based on the highest existing ID
// (not just memberCount), so removing then adding members never reissues an ID.
static string generateNextMemberID(const Member members[], int memberCount) {
    int maxNum = 1000;
    for (int i = 0; i < memberCount; ++i) {
        const string& id = members[i].memberID;
        if (id.size() > 1 && id[0] == 'M') {
            try {
                int num = stoi(id.substr(1));
                if (num > maxNum) maxNum = num;
            }
            catch (...) {
                // Non-numeric suffix (e.g. legacy/custom ID) - ignore it.
            }
        }
    }
    return "M" + to_string(maxNum + 1);
}

// 5.1.1 Register Member
void registerMember(Member members[], int& memberCount) {
    clearScreen();
    displayAppHeader("REGISTER NEW MEMBER");

    if (memberCount >= MAX_MEMBERS) {
        printBoxLine("[!] Member database is full.", "center");
        printBoxBorder();
        pauseScreen();
        return;
    }

    string fullName, password, email, phone;
    printBoxLine("=== REGISTER NEW MEMBER ===", "left", 6);
    printBoxLine("(Enter 0 as your name at any time to cancel and go back)", "left", 6);
    printBoxBorder();

    cout << string(getLeftMargin() + 6, ' ') << "Enter Full Name           : ";
    cin >> fullName;
    if (fullName == "0") {
        return;
    }

    cout << string(getLeftMargin() + 6, ' ') << "Enter Password            : ";
    password = readMaskedInput();

    while (true) {
        cout << string(getLeftMargin() + 6, ' ') << "Enter Email               : ";
        cin >> email;
        if (isValidEmail(email)) break;
        cout << string(getLeftMargin() + 6, ' ') << "[ERROR] Invalid email format! (e.g., user@example.com)\n";
    }

    while (true) {
        cout << string(getLeftMargin() + 6, ' ') << "Enter Contact Number      : ";
        cin >> phone;
        if (isValidPhone(phone)) break;
        cout << string(getLeftMargin() + 6, ' ') << "[ERROR] Phone number must be 10-11 digits, numbers only.\n";
    }

    string newID = generateNextMemberID(members, memberCount);
    members[memberCount] = { newID, fullName, password, "2026-12-31", email, phone, 0 };
    memberCount++;
    DataStore::saveMembers(members, memberCount);

    clearScreen();
    displayAppHeader("REGISTRATION SUCCESSFUL");
    printBoxLine("Member " + fullName + " registered successfully!", "center");
    printBoxLine("Your Member ID: " + newID, "center");
    printBoxBorder();
    pauseScreen();
}

// 5.1.3 Forgot Password (Added exist check)
void forgotPassword(Member members[], int memberCount) {
    clearScreen();
    displayAppHeader("FORGOT PASSWORD");

    string email;
    cout << string(getLeftMargin() + 6, ' ') << "Enter your registered Email: ";
    cin >> email;

    int foundIdx = -1;
    for (int i = 0; i < memberCount; ++i) {
        if (members[i].email == email) {
            foundIdx = i;
            break;
        }
    }

    if (foundIdx == -1) {
        clearScreen();
        displayAppHeader("ERROR");
        printBoxLine("[ERROR] Registered email not found!", "center");
        printBoxBorder();
        pauseScreen();
        return;
    }

    string code, newPassword;
    cout << string(getLeftMargin() + 6, ' ') << "Enter Security Verification Code sent to email: ";
    cin >> code;
    cout << string(getLeftMargin() + 6, ' ') << "Enter New Password: ";
    newPassword = readMaskedInput();

    members[foundIdx].password = newPassword;
    DataStore::saveMembers(members, memberCount);

    clearScreen();
    displayAppHeader("PASSWORD UPDATED");
    printBoxLine("Password updated successfully! Please login with your new password.", "center");
    printBoxBorder();
    pauseScreen();
}

// 5.1.2 Login User (Refactored to handle Admin & Customer verification)
string loginUser(const Member members[], int memberCount, string& loggedID) {
    setCurrentRoleLabel(""); // no role while on the login screen (e.g. after a logout)
    while (true) {
        clearScreen();
        displayAppHeader("LOGIN SYSTEM");
        printBoxLine("[1] Login", "left", 6);
        printBoxLine("[2] Forgot Password", "left", 6);
        printBoxLine("[0] Exit System", "left", 6);
        printBoxBorder();

        int choice;
        cout << "\n" << string(getLeftMargin() + 6, ' ') << "Select option: ";
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(10000, '\n');
            continue;
        }

        if (choice == 1) {
            string username, password;
            cout << string(getLeftMargin() + 6, ' ') << "Enter User ID / Username: ";
            cin >> username;
            cout << string(getLeftMargin() + 6, ' ') << "Enter Password          : ";
            password = readMaskedInput();

            // Check Admin Credentials
            if (username == "admin" && password == "admin123") {
                setCurrentRoleLabel("ADMIN");
                clearScreen();
                displayAppHeader("LOGIN SUCCESS");
                printBoxLine("Login Successful as Admin.", "center");
                printBoxBorder();
                pauseScreen();
                loggedID = "admin";
                return "admin";
            }

            // Check Customer / Member Credentials
            for (int i = 0; i < memberCount; ++i) {
                if ((members[i].memberID == username || members[i].fullName == username) && members[i].password == password) {
                    setCurrentRoleLabel("CUSTOMER (" + members[i].memberID + ")");
                    clearScreen();
                    displayAppHeader("LOGIN SUCCESS");
                    printBoxLine("Login Successful as Customer.", "center");
                    printBoxBorder();
                    pauseScreen();
                    loggedID = members[i].memberID;
                    return "customer";
                }
            }

            clearScreen();
            displayAppHeader("LOGIN FAILED");
            printBoxLine("[ERROR] Invalid username or password! Please try again.", "center");
            printBoxBorder();
            pauseScreen();
        }
        else if (choice == 2) {
            forgotPassword(const_cast<Member*>(members), memberCount);
        }
        else if (choice == 0) {
            return "";
        }
    }
}

// 5.1.5 Edit Member Profile fields, given a resolved index (shared by
// self-service "Edit Profile" and admin "Edit Member" from the members list)
static void editMemberFields(Member members[], int memberCount, int mIdx) {
    clearScreen();
    displayAppHeader("EDIT MEMBER PROFILE");
    printBoxLine("Editing: " + members[mIdx].fullName + " (" + members[mIdx].memberID + ")", "left", 6);
    printBoxDivider();
    printBoxLine("[1] Update Email", "left", 6);
    printBoxLine("[2] Update Contact Number", "left", 6);
    printBoxLine("[3] Change Password", "left", 6);
    printBoxLine("[0] Cancel", "left", 6);
    printBoxBorder();

    int choice;
    cout << "\n" << string(getLeftMargin() + 6, ' ') << "Select choice: ";
    cin >> choice;

    if (choice == 1) {
        cout << "\n" << string(getLeftMargin() + 6, ' ') << "Current Email: " << members[mIdx].email << "\n";
        string newEmail;
        cout << string(getLeftMargin() + 6, ' ') << "Enter New Email: ";
        cin >> newEmail;

        if (!isValidEmail(newEmail)) {
            cout << "\n" << string(getLeftMargin() + 6, ' ') << "[ERROR] Invalid email format! (e.g., user@example.com)\n";
        } else if (newEmail == members[mIdx].email) {
            cout << "\n" << string(getLeftMargin() + 6, ' ') << "[ERROR] New email must be different from the current email.\n";
        } else {
            members[mIdx].email = newEmail;
            DataStore::saveMembers(members, memberCount);
            cout << "\n" << string(getLeftMargin() + 6, ' ') << "Email updated to: " << newEmail << "\n";
        }
        pauseScreen();
    }
    else if (choice == 2) {
        string newPhone;
        cout << string(getLeftMargin() + 6, ' ') << "Enter New Contact Number: ";
        cin >> newPhone;
        members[mIdx].phone = newPhone;
        DataStore::saveMembers(members, memberCount);
        cout << "\n" << string(getLeftMargin() + 6, ' ') << "Contact Number updated to: " << newPhone << "\n";
        pauseScreen();
    }
    else if (choice == 3) {
        string newPass;
        cout << string(getLeftMargin() + 6, ' ') << "Enter New Password: ";
        newPass = readMaskedInput();
        members[mIdx].password = newPass;
        DataStore::saveMembers(members, memberCount);
        cout << "\n" << string(getLeftMargin() + 6, ' ') << "Password changed successfully.\n";
        pauseScreen();
    }
    else if (choice == 0) {
        return;
    }
}

void editMemberProfile(Member members[], int memberCount, const string& currentMemberID) {
    int mIdx = -1;
    for (int i = 0; i < memberCount; ++i) {
        if (members[i].memberID == currentMemberID) {
            mIdx = i;
            break;
        }
    }
    if (mIdx == -1) {
        clearScreen();
        displayAppHeader("ERROR");
        printBoxLine("[ERROR] Profile data not found.", "center");
        printBoxBorder();
        pauseScreen();
        return;
    }
    editMemberFields(members, memberCount, mIdx);
}

// Edit an arbitrary member picked from the All Members List (admin action):
// show the full member list, then let the admin type which Member ID to edit.
static void editMemberByID(Member members[], int memberCount) {
    clearScreen();
    displayAppHeader("EDIT MEMBER");

    if (memberCount == 0) {
        printBoxLine("[!] No members to edit.", "center");
        printBoxBorder();
        pauseScreen();
        return;
    }

    for (int i = 0; i < memberCount; ++i) {
        printBoxLine("ID: " + members[i].memberID + " | Name: " + members[i].fullName, "left", 6);
    }
    printBoxBorder();

    string targetID;
    cout << "\n" << string(getLeftMargin() + 6, ' ') << "Enter Member ID to Edit (or '0' to Cancel): ";
    cin >> targetID;

    if (targetID == "0") return;

    int mIdx = -1;
    for (int i = 0; i < memberCount; ++i) {
        if (members[i].memberID == targetID) { mIdx = i; break; }
    }

    if (mIdx == -1) {
        clearScreen();
        displayAppHeader("NOT FOUND");
        printBoxLine("[ERROR] Member ID does not exist.", "center");
        printBoxBorder();
        pauseScreen();
        return;
    }

    editMemberFields(members, memberCount, mIdx);
}

// 5.1.4 View Member Profile
void viewMemberProfile(Member members[], int memberCount, const string& currentMemberID) {
    clearScreen();
    displayAppHeader("MY PROFILE");

    int mIdx = -1;
    for (int i = 0; i < memberCount; ++i) {
        if (members[i].memberID == currentMemberID) {
            mIdx = i;
            break;
        }
    }

    if (mIdx != -1) {
        printBoxLine("ID    : " + members[mIdx].memberID, "left", 6);
        printBoxLine("Name  : " + members[mIdx].fullName, "left", 6);
        printBoxLine("Email : " + members[mIdx].email, "left", 6);
        printBoxLine("Phone : " + members[mIdx].phone, "left", 6);
        printBoxLine("Membership Status: Active", "left", 6);
    }
    else {
        printBoxLine("[ERROR] Profile data not found.", "center");
    }

    printBoxDivider();
    printBoxLine("[1] Edit Profile", "left", 6);
    printBoxLine("[0] Back", "left", 6);
    printBoxBorder();

    int choice;
    cout << "\n" << string(getLeftMargin() + 6, ' ') << "Select choice: ";
    cin >> choice;

    if (choice == 1) {
        editMemberProfile(members, memberCount, currentMemberID);
    }
    else if (choice == 0) {
        return;
    }
}

// 5.1.7 Remove Member (used by viewAllMembers) - arrow-key select, then confirm
static void removeMember(Member members[], int& memberCount) {
    if (memberCount == 0) {
        clearScreen();
        displayAppHeader("REMOVE MEMBER");
        printBoxLine("[!] No members to remove.", "center");
        printBoxBorder();
        pauseScreen();
        return;
    }

    vector<string> options;
    for (int i = 0; i < memberCount; ++i) {
        options.push_back(members[i].memberID + " - " + members[i].fullName);
    }
    int selIdx = selectFromList("SELECT MEMBER TO REMOVE", options);
    if (selIdx == -1) return; // canceled - back to the members list

    string memberID = members[selIdx].memberID;

    clearScreen();
    displayAppHeader("REMOVE MEMBER");
    cout << string(getLeftMargin() + 6, ' ') << "Are you sure you want to remove Member ID " << memberID << "? (Y/N): ";
    char confirm;
    cin >> confirm;
    if (confirm == 'Y' || confirm == 'y') {
        bool found = false;
        for (int i = 0; i < memberCount; ++i) {
            if (members[i].memberID == memberID) {
                for (int j = i; j < memberCount - 1; ++j) {
                    members[j] = members[j + 1];
                }
                memberCount--;
                found = true;
                break;
            }
        }
        if (found) {
            DataStore::saveMembers(members, memberCount);
            cout << string(getLeftMargin() + 6, ' ') << "Member ID " << memberID << " removed successfully.\n";
        }
        else {
            cout << string(getLeftMargin() + 6, ' ') << "Member ID not found.\n";
        }
    }
    else {
        cout << string(getLeftMargin() + 6, ' ') << "Removal cancelled.\n";
    }
    pauseScreen();
}

// 5.1.6 View All Members
void viewAllMembers(Member members[], int& memberCount) {
    int choice;
    do {
        clearScreen();
        displayAppHeader("ALL MEMBERS LIST");

        for (int i = 0; i < memberCount; ++i) {
            printBoxLine("ID: " + members[i].memberID + " | Name: " + members[i].fullName + " | Status: Active", "left", 6);
        }
        printBoxDivider();
        printBoxLine("[1] Add Member", "left", 6);
        printBoxLine("[2] Remove Member", "left", 6);
        printBoxLine("[3] Edit Member", "left", 6);
        printBoxLine("[0] Back", "left", 6);
        printBoxBorder();

        cout << "\n" << string(getLeftMargin() + 6, ' ') << "Select choice: ";
        cin >> choice;

        if (choice == 1) {
            registerMember(members, memberCount);
        }
        else if (choice == 2) {
            removeMember(members, memberCount);
        }
        else if (choice == 3) {
            editMemberByID(members, memberCount);
        }
    } while (choice != 0);
}

// Auto-generate the next "MS###" master ID, based on the highest existing ID
static string generateNextMasterID(const Master masters[], int masterCount) {
    int maxNum = 0;
    for (int i = 0; i < masterCount; ++i) {
        const string& id = masters[i].masterID;
        if (id.size() > 2 && id[0] == 'M' && id[1] == 'S') {
            try {
                int num = stoi(id.substr(2));
                if (num > maxNum) maxNum = num;
            }
            catch (...) {
                // Non-numeric suffix - ignore it.
            }
        }
    }
    string numStr = to_string(maxNum + 1);
    while (numStr.length() < 3) numStr = "0" + numStr;
    return "MS" + numStr;
}

// 5.1.10a Add Master (Admin only) - now lives on the master list screen, not
// inside master details.
static void addMaster(Master masters[], int& masterCount) {
    clearScreen();
    displayAppHeader("ADD NEW MASTER");

    if (masterCount >= MAX_MASTERS) {
        printBoxLine("[!] Master limit reached.", "center");
        printBoxBorder();
        pauseScreen();
        return;
    }

    cin.ignore(10000, '\n');
    string masterName, specialization, expStr;
    cout << string(getLeftMargin() + 6, ' ') << "Enter Master Name (or 0 to cancel): ";
    getline(cin, masterName);
    if (masterName == "0") return;

    while (true) {
        cout << string(getLeftMargin() + 6, ' ') << "Enter Specialization (letters only): ";
        getline(cin, specialization);
        if (isValidSpecialization(specialization)) break;
        cout << string(getLeftMargin() + 6, ' ') << "[ERROR] Specialization must contain letters only.\n";
    }

    int experience = 0;
    while (true) {
        cout << string(getLeftMargin() + 6, ' ') << "Enter Experience (Years, numbers only): ";
        getline(cin, expStr);
        if (isValidExperience(expStr)) {
            experience = stoi(expStr);
            break;
        }
        cout << string(getLeftMargin() + 6, ' ') << "[ERROR] Experience must be a number (0-99).\n";
    }

    string newID = generateNextMasterID(masters, masterCount);
    masters[masterCount] = { newID, masterName, specialization, experience, 5.0 };
    masterCount++;
    DataStore::saveMasters(masters, masterCount);

    clearScreen();
    displayAppHeader("MASTER ADDED");
    printBoxLine("[SUCCESS] Master " + masterName + " (" + newID + ") added successfully!", "center");
    printBoxBorder();
    pauseScreen();
}

// 5.1.10b Remove Master (Admin only) - arrow-key select, then confirm; Esc/
// canceling the picker just returns to the master list (no ID typing needed).
static void removeMaster(Master masters[], int& masterCount) {
    if (masterCount == 0) {
        clearScreen();
        displayAppHeader("REMOVE MASTER");
        printBoxLine("[!] No masters to remove.", "center");
        printBoxBorder();
        pauseScreen();
        return;
    }

    vector<string> options;
    for (int i = 0; i < masterCount; ++i) {
        options.push_back(masters[i].masterID + " - " + masters[i].name);
    }
    int idx = selectFromList("SELECT MASTER TO REMOVE", options);
    if (idx == -1) return; // canceled - back to the master list

    clearScreen();
    displayAppHeader("REMOVE MASTER");
    cout << string(getLeftMargin() + 6, ' ') << "Remove " << masters[idx].name << " (" << masters[idx].masterID << ")? (Y/N): ";
    char confirm;
    cin >> confirm;
    if (confirm == 'Y' || confirm == 'y') {
        for (int j = idx; j < masterCount - 1; ++j) {
            masters[j] = masters[j + 1];
        }
        masterCount--;
        DataStore::saveMasters(masters, masterCount);
        cout << string(getLeftMargin() + 6, ' ') << "Master removed successfully.\n";
    }
    else {
        cout << string(getLeftMargin() + 6, ' ') << "Removal cancelled.\n";
    }
    pauseScreen();
}

// 5.1.9 View Master Details - Left/Right arrows cycle between masters
void viewMasterDetails(const Master masters[], int masterCount, int startIndex) {
    if (masterCount == 0) {
        clearScreen();
        displayAppHeader("MASTER DETAILS");
        printBoxLine("[!] No masters available.", "center");
        printBoxBorder();
        pauseScreen();
        return;
    }

    int idx = (startIndex >= 0 && startIndex < masterCount) ? startIndex : 0;

    while (true) {
        clearScreen();
        displayAppHeader("MASTER DETAILS (" + to_string(idx + 1) + "/" + to_string(masterCount) + ")");
        printBoxLine("Master ID     : " + masters[idx].masterID, "left", 6);
        printBoxLine("Name          : " + masters[idx].name, "left", 6);
        printBoxLine("Specialization: " + masters[idx].specialization, "left", 6);
        printBoxLine("Experience    : " + to_string(masters[idx].experience) + " Years", "left", 6);
        printBoxLine("Rating        : " + to_string(masters[idx].rating).substr(0, 3) + " / 5.0", "left", 6);
        printBoxDivider();
        printBoxLine("Left/Right = switch master | 0 = Back", "center");
        printBoxBorder();

#ifdef _WIN32
        int key = _getch();
        if (key == 0 || key == 224) {
            int key2 = _getch();
            if (key2 == 75) { // Left
                idx = (idx - 1 + masterCount) % masterCount;
            }
            else if (key2 == 77) { // Right
                idx = (idx + 1) % masterCount;
            }
        }
        else if (key == 27 || key == '0') {
            return;
        }
#else
        cout << "\n" << string(getLeftMargin() + 6, ' ') << "[1] Next  [2] Previous  [0] Back: ";
        int choice;
        cin >> choice;
        if (choice == 1) idx = (idx + 1) % masterCount;
        else if (choice == 2) idx = (idx - 1 + masterCount) % masterCount;
        else if (choice == 0) return;
#endif
    }
}

// 5.1.8 View Master List
void viewMasterList(const string& role, Master masters[], int& masterCount) {
    bool isAdmin = (role == "admin");
    int choice;
    do {
        clearScreen();
        displayAppHeader("MASTER LIST");

        for (int i = 0; i < masterCount; ++i) {
            printBoxLine(to_string(i + 1) + ". Master ID: " + masters[i].masterID + " | Name: " + masters[i].name + " | Specialization: " + masters[i].specialization, "left", 6);
        }
        printBoxDivider();
        printBoxLine("[1] View Master Details", "left", 6);
        if (isAdmin) {
            printBoxLine("[2] Add Master", "left", 6);
            printBoxLine("[3] Remove Master", "left", 6);
        }
        printBoxLine("[0] Back", "left", 6);
        printBoxBorder();

        cout << "\n" << string(getLeftMargin() + 6, ' ') << "Select choice: ";
        cin >> choice;

        if (choice == 1) {
            viewMasterDetails(masters, masterCount, 0);
        }
        else if (choice == 2 && isAdmin) {
            addMaster(masters, masterCount);
        }
        else if (choice == 3 && isAdmin) {
            removeMaster(masters, masterCount);
        }
    } while (choice != 0);
}

// Menu Customer Management (Submenu for Member & Master operations)
void menuCustomerManagement(const string& role, Member members[], int& memberCount, const string& currentMemberID, Master masters[], int& masterCount) {
    bool isAdmin = (role == "admin");
    int choice;
    do {
        clearScreen();
        displayAppHeader("MEMBER MANAGEMENT MENU");
        if (isAdmin) {
            printBoxLine("[1] View All Members List", "left", 6);
        }
        else {
            printBoxLine("[1] View My Profile", "left", 6);
        }
        printBoxLine("[2] View Master List", "left", 6);
        printBoxLine("[0] Back", "left", 6);
        printBoxBorder();

        cout << "\n" << string(getLeftMargin() + 6, ' ') << "Select choice: ";
        cin >> choice;

        if (choice == 1) {
            if (isAdmin) {
                viewAllMembers(members, memberCount);
            }
            else {
                viewMemberProfile(members, memberCount, currentMemberID);
            }
        }
        else if (choice == 2) {
            viewMasterList(role, masters, masterCount);
        }
        else if (choice == 0) {
            return;
        }
    } while (choice != 0);
}

// 5.1.11 Renew Membership (Admin action on a member's expiry date)
void renewMembership(Member members[], int memberCount) {
    clearScreen();
    displayAppHeader("RENEW MEMBERSHIP");

    string today = getCurrentDateStr();

    printBoxLine("--- CURRENT MEMBER STATUS & EXPIRY LIST ---", "center");
    printBoxLine("System Today's Date: " + today, "center");
    printBoxDivider();
    string header = " Member ID | Full Name        | Expiry Date   | Status";
    printBoxLine(header, "left", 6);
    printBoxDivider();

    for (int i = 0; i < memberCount; ++i) {
        string status = (members[i].expiryDate >= today) ? "ACTIVE" : "EXPIRED";
        string row = " " + members[i].memberID + string(10 - (int)members[i].memberID.length(), ' ') + "| "
            + members[i].fullName + string(17 - (int)members[i].fullName.length(), ' ') + "| "
            + members[i].expiryDate + string(14 - (int)members[i].expiryDate.length(), ' ') + "| "
            + status;
        printBoxLine(row, "left", 6);
    }
    printBoxBorder();

    string targetID;
    cout << "\n" << string(getLeftMargin() + 6, ' ') << "Enter Member ID to Renew (or '0' to Cancel): ";
    cin >> targetID;

    if (targetID == "0") return;

    int mIdx = -1;
    for (int i = 0; i < memberCount; ++i) {
        if (members[i].memberID == targetID) { mIdx = i; break; }
    }

    if (mIdx == -1) {
        clearScreen();
        displayAppHeader("NOT FOUND");
        printBoxLine("[ERROR] Member ID does not exist.", "center");
        printBoxBorder();
        pauseScreen();
        return;
    }

    string newExpiry;
    cout << string(getLeftMargin() + 6, ' ') << "Enter New Expiry Date (YYYY-MM-DD): ";
    cin >> newExpiry;

    if (newExpiry.length() != 10 || newExpiry[4] != '-' || newExpiry[7] != '-') {
        clearScreen();
        displayAppHeader("INVALID FORMAT");
        printBoxLine("[ERROR] Invalid date format! Please use YYYY-MM-DD.", "center");
        printBoxBorder();
        pauseScreen();
        return;
    }

    if (newExpiry <= today) {
        clearScreen();
        displayAppHeader("INVALID DATE");
        printBoxLine("[ERROR] Expiry date MUST be a future date!", "center");
        printBoxLine("Today is: " + today + " | Input date: " + newExpiry, "center");
        printBoxBorder();
        pauseScreen();
        return;
    }

    members[mIdx].expiryDate = newExpiry;
    DataStore::saveMembers(members, memberCount);

    clearScreen();
    displayAppHeader("RENEWAL SUCCESSFUL");
    printBoxLine(" ", "center");
    printBoxLine("[SUCCESS] Membership renewed for " + members[mIdx].fullName + " (" + targetID + ")", "center");
    printBoxLine("Previous Expiry Date : " + today, "left", 20);
    printBoxLine("Updated Expiry Date  : " + newExpiry, "left", 20);
    printBoxLine("Account Status       : ACTIVE", "left", 20);
    printBoxLine(" ", "center");
    printBoxBorder();
    pauseScreen();
}
